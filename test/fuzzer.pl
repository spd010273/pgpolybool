#!/usr/bin/perl

use strict;
use warnings;

use DBI;
use Carp;
use Readonly;
use English qw( -no_match_vars );
use Time::HiRes qw( gettimeofday tv_interval );
use Data::Dumper;
use Getopt::Std;

Readonly my $TEST_DATABASE           => '__pgp_testing__';
Readonly my $SERIES_BOUND            => 10000;
Readonly my $MAX_NUM_POINTS          => 500;
Readonly my $NUMBER_OF_TESTS         => 10000;
Readonly my $CONNECTION_STRING       => "dbi:Pg:dbname=$TEST_DATABASE;host=localhost;port=5432";
Readonly my $POSTGRES_CONN_STRING    => 'dbi:Pg:dbname=postgres;host=localhost;port=5432';
Readonly my $OUTPUT_UNIT             => 'ms'; # or s or ms
Readonly my $SQLSTATE_CONN_EXCEPTION => '08000';
Readonly my $TEST_POLY_DATA => <<"END_SQL";
CREATE TEMP TABLE tt_poly_data AS
(
    WITH tt_series_bound AS
    (
        SELECT ${SERIES_BOUND}::INTEGER AS series_bound
    ),
    tt_num_points AS
    (
        SELECT ${MAX_NUM_POINTS}::INTEGER AS num_points
    )
    tt_intersecting_poly_data AS
    (
        SELECT generate_series( -series_bound, -1, 1 ) AS p1x,
               generate_series( -series_bound, -1, 1 ) AS p1y,
               generate_series( series_bound, 1, -1 ) AS p2x,
               generate_series( -series_bound, -1, 1 ) AS p2y,
               generate_series( series_bound, 1, -1 ) AS p3x,
               generate_series( series_bound, 1, -1 ) AS p3y,
               generate_series( -series_bound, -1, 1 ) AS p4x,
               generate_series( series_bound, 1, -1 ) AS p4y
          FROM tt_series_bound
    ),
    tt_nonintersecting_poly_data AS
    (
        WITH tt_x_series_setup AS
        (
            SELECT generate_series( -series_bound, series_bound - 1, 1 ) AS x
              FROM tt_series_bound
        )
            SELECT ttx.x AS p1x,
                   -tts.series_bound AS p1y,
                   ttx.x + 1 AS p2x,
                   -tts.series_bound AS p2y,
                   ttx.x + 1 AS p3x,
                   tts.series_bound AS p3y,
                   ttx.x AS p4x,
                   tts.series_bound AS p4y
              FROM tt_x_series_setup ttx
        INNER JOIN tt_series_bound tts
                ON TRUE
    ),
    tt_large_point_set AS
    (
        SELECT polygon(
                   ttnp.num_points,
                   circle(
                       point(
                           generate_series( -( ttsb.series_bound / 2 ), ( ttsb.series_bound / 2 ), 1 ),
                           0
                       ),
                       ( ttsb.series_bound / 2 )
                   )
               ) AS poly,
               ttnp.num_points AS num_pts,
               TRUE AS intersecting
          FROM tt_series_bound ttsb
    INNER JOIN tt_num_points ttnp
            ON TRUE
    ),
    tt_set_union AS
    (
        SELECT p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y,
               TRUE AS intersecting,
               4 as num_pts
          FROM tt_intersecting_poly_data
         UNION ALL
        SELECT p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y,
               FALSE AS intersecting,
               4 AS num_pts
          FROM tt_nonintersecting_poly_data
    ),
    tt_point_transform AS
    (
        SELECT point( p1x, p1y ) AS p1,
               point( p2x, p2y ) AS p2,
               point( p3x, p3y ) AS p3,
               point( p4x, p4y ) AS p4,
               num_pts,
               intersecting
          FROM tt_set_union
    )
        SELECT ( '(' || p1::VARCHAR || ',' || p2::VARCHAR || ',' || p3::VARCHAR || ',' || p4::VARCHAR || ')' )::POLYGON AS poly,
               num_pts,
               intersecting
          FROM tt_point_transform
         UNION ALL
        SELECT poly,
               num_pts,
               intersecting
)
END_SQL

Readonly my $DBNAME_CHECK_QUERY => <<END_SQL;
    SELECT COUNT(*) AS count
      FROM pg_database
     WHERE datname = ?
END_SQL

Readonly my $MARTINEZ_TESTS => {
    difference => {
        query           => 'SELECT fn_subtract_polygons( ?, ? ) AS result',
        parameter_count => 2,
        intersecting    => 1,
    },
    intersect => {
        query           => 'SELECT fn_intersect_polygons( ?, ? ) AS result',
        parameter_count => 2,
        intersecting    => 1,
    },
    union => {
        query           => 'SELECT fn_union_polygons( ?, ? ) AS result',
        parameter_count => 2,
    },
    xor => {
        query           => 'SELECT fn_xor_polygons( ?, ? ) AS result',
        parameter_count => 2,
    },
};

Readonly my $CACHE_TEST_DATA => <<END_SQL;
    SELECT poly
      FROM tt_poly_data
     WHERE __WHERE__
END_SQL

if(
    $OUTPUT_UNIT ne 's' &&
    $OUTPUT_UNIT ne 'ms' &&
    $OUTPUT_UNIT ne 'us'
   )
{
    croak 'Invalid setting for $OUTPUT_UNIT, use either s, ms, or us';
}

our( $opt_C );

unless( getopts( 'C' ) )
{
    croak 'Invalid arguments';
}

my $make_check = $opt_C;
if( defined $make_check and $make_check )
{
    $make_check = 1;
}
else
{
    $make_check = 0;
}

# Setup
my $pg_handle = DBI->connect(
    $POSTGRES_CONN_STRING,
    'postgres',
    undef
);

unless( $pg_handle )
{
    croak( 'Failed to connect to database' );
}

my $sth = $pg_handle->prepare( $DBNAME_CHECK_QUERY );
$sth->bind_param( 1, $TEST_DATABASE );

unless( $sth->execute() )
{
    croak( "Failed to check state of $TEST_DATABASE" );
}

my $row = $sth->fetchrow_hashref();

if( $row->{count} == 0 )
{
    unless( $pg_handle->do( "CREATE DATABASE \"$TEST_DATABASE\"" ) )
    {
        croak( 'Failed to create database for tests' );
    }
}
else
{
    carp( "Test database '$TEST_DATABASE' already exists" );

    unless( $pg_handle->do( "DROP DATABASE \"$TEST_DATABASE\"" ) )
    {
        croak( "Failed to remove testing database '$TEST_DATABASE'" );
    }

    unless( $pg_handle->do( "CREATE DATABASE \"$TEST_DATABASE\"" ) )
    {
        croak( "Failed to create testing database '$TEST_DATABASE'" );
    }
}

$sth->finish();
$pg_handle->disconnect();

my $handle = DBI->connect(
    $CONNECTION_STRING,
    'postgres',
    undef
);

unless( $handle )
{
    croak( 'Failed to connect to testing database' );
}

unless( $handle->do( 'CREATE EXTENSION pgpolybool' ) )
{
    croak( 'Failed to install pgpolybool on test database' );
}

# Generate test data

unless( $handle->do( $TEST_POLY_DATA ) )
{
    croak( 'Failed to generate test polygon data' );
}

foreach my $key ( ( sort { $a cmp $b } keys %$MARTINEZ_TESTS ) )
{
    print "Benchmarking $key:\n";

    my $timed_query = $MARTINEZ_TESTS->{$key}->{query};
    my $cache_query = $CACHE_TEST_DATA;
    my $where       = 'TRUE';

    if( defined $MARTINEZ_TESTS->{$key}->{intersecting} )
    {
        if( $MARTINEZ_TESTS->{$key}->{intersecting} )
        {
            $where = 'intersecting IS TRUE';
        }
        else
        {
            $where = 'intersecting IS FALSE';
        }
    }

    print "  Caching test data...";
    $cache_query =~ s/__WHERE__/$where/;
    my $sth = $handle->prepare( $cache_query );

    unless( $sth )
    {
        croak 'Failed to prepare caching query';
    }

    unless( $sth->execute() )
    {
        croak 'Failed to execute caching query';
    }

    my $TEST_DATA = [];
    while( my $row = $sth->fetchrow_hashref )
    {
        push( @$TEST_DATA, $row->{poly} );
    }

    $sth->finish();
    print " Done.\n  Running benchmark...";
    my $RESULTS = [];
    $sth = $handle->prepare( $timed_query );

    unless( $sth )
    {
        croak 'Failed to prepare query under test';
    }

    my $max_delta  = -9999999;
    my $min_delta  = 9999999;
    my $test_sz    = scalar( @$TEST_DATA );
    my $null_count = 0;

    for( my $i = 0; $i < $NUMBER_OF_TESTS; $i++ )
    {
        my $params  = [];

        for( my $j = 0; $j < $MARTINEZ_TESTS->{$key}->{parameter_count}; $j++ )
        {
            push( @$params, $TEST_DATA->[rand $test_sz] );
        }

        my $param_ind = 1;
        foreach my $param( @$params )
        {
            $sth->bind_param( $param_ind, $param );
            $param_ind++;
        }

        my $test_start = [gettimeofday()];

        unless( $sth->execute() )
        {
            my $sqlstate = $handle->state;
            if( $sqlstate eq $SQLSTATE_CONN_EXCEPTION )
            {
                my $crashed_params = Dumper( @$params );
                print "\n\n\n";
                print <<"ERROR";
===============================================================================
pgpolybool seems to have crashed while the benchmark was running the following
command:

$timed_query

with the parameters

$crashed_params

Please check the server error logs. If the server has reported a SIGSEGV,
please forward this information along to the pgpolybool bitbucket.

Thank you.
===============================================================================
ERROR
            }

            croak "Failed to execute test query ($sqlstate)";
        }

        my $test_end = [gettimeofday()];
        my $delta    = ( tv_interval( $test_start, $test_end ) * 1000 );

        if( $OUTPUT_UNIT eq 'ms' )
        {
            $delta = $delta * 1000;
        }
        elsif( $OUTPUT_UNIT eq 'us' )
        {
            $delta = $delta * 1000000;
        }

        my $row = $sth->fetchrow_hashref();

        unless( $row->{result} )
        {
            $null_count++;
        }

        push( @$RESULTS, $delta );

        $max_delta = $delta if( $delta > $max_delta );
        $min_delta = $delta if( $delta < $min_delta );
    }

    print " Done.\n";
    my $sum   = 0;
    my $count = 0;
    foreach my $result (@$RESULTS)
    {
        $sum += $result;
        $count++;
    }

    my $average = ( $sum / $count );
    my $std_dev_partial_sum = 0;
    foreach my $result(@$RESULTS)
    {
        $std_dev_partial_sum += ( $result - $average ) * ( $result - $average );
    }

    my $std_dev = sqrt( ( 1 / ( $count - 1 ) ) * $std_dev_partial_sum );
    unless( $make_check )
    {
    print <<"STATS";
  Statistics for this call:
    Average: $average $OUTPUT_UNIT
    Std Dev: $std_dev $OUTPUT_UNIT^2
    Min:     $min_delta $OUTPUT_UNIT
    Max:     $max_delta $OUTPUT_UNIT
    Count:   $count
    NULLs:   $null_count
STATS
    }
    else
    {
        print "  PASSED\n";
    }
}

exit 0 ;
