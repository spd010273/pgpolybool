#!/usr/bin/perl

use warnings;
use strict;

use DBI;
use Carp;
use JSON;
use Readonly;
use English qw( -no_match_vars );
use Cwd qw( abs_path );

Readonly my $TEST_DIR             => 'test/sql';
Readonly my $TEST_DATABASE        => '__pgp_testing__';
Readonly my $CONNECTION_STRING    => "dbi:Pg:dbname=$TEST_DATABASE;host=localhost;port=5432";
Readonly my $POSTGRES_CONN_STRING => 'dbi:Pg:dbname=postgres;host=localhost;port=5432';
Readonly my $DBNAME_CHECK_QUERY   => <<END_SQL;
    SELECT COUNT(*) AS count
      FROM pg_database
     WHERE datname = ?
END_SQL

# Prepare a database for use
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
        croak( 'Failed to create database for tests' );
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

unless( $handle->do( 'DROP EXTENSION IF EXISTS pgpolybool' ) ) # not really needed
{
    croak( 'Failed to drop extension' );
}

unless( $handle->do( 'CREATE EXTENSION pgpolybool' ) )
{
    croak( 'Failed to create extension' );
}

## Locate and open test casts
my $path = abs_path( $PROGRAM_NAME );
my $ABS_TEST_PATH = $path;
$ABS_TEST_PATH =~ s/pgpolybool\/.*$//;
$ABS_TEST_PATH .= "pgpolybool/$TEST_DIR";

unless( opendir( TESTDIR, $ABS_TEST_PATH ) )
{
    croak( "Failed to open test directory '$ABS_TEST_PATH': $OS_ERROR" );
}

my @files = readdir( TESTDIR );
closedir( TESTDIR );

my $test_results    = {};
my $test_count      = 0;
my $max_file_length = 0; # used for displaying results

foreach my $file ( sort { $a cmp $b } @files )
{
    if( $file =~ m/^\.$/ || $file =~ m/^\.\.$/ )
    {
        next;
    }

    unless( $file =~ m/\.json$/i )
    {
        next;
    }

    if( length( $file ) > $max_file_length )
    {
        $max_file_length = length( $file );
    }

    my $test_fh;
    my $test_name = $file;
    $test_name =~ s/\.json$//;

    unless( open( $test_fh, '<:encoding(UTF-8)', "${ABS_TEST_PATH}/${file}" ) )
    {
        croak( "Failed to open '${ABS_TEST_PATH}/${file}': $OS_ERROR" );
    }

    my $file_data = '';

    while( my $line = <$test_fh> )
    {
        chomp( $line );
        $file_data .= $line;
    }

    close( $test_fh );
    my $tests = decode_json( $file_data );

    unless( $tests && ref( $tests ) eq 'HASH' )
    {
        croak( "Expected JSON object for tests" );
    }

    print "Running $file...\n";

    foreach my $test_id( sort { $a <=> $b } keys %$tests )
    {
        my $test_command = $tests->{$test_id};
        $test_count++;

        $sth = $handle->prepare( $test_command );

        unless( $sth )
        {
            croak( "Failed to prepare test $test_id" );
        }

        unless( $sth->execute() )
        {
            croak( "Failed to execute test $test_id" );
        }

        my $result = $sth->fetchrow_arrayref();

        if( !defined( $result->[0] ) || $result->[0] == 0 )
        {
            print "   Test $test_id failed.\n";
            $test_results->{$file}->{$test_id} = 'Failed';
        }
        else
        {
            $test_results->{$file}->{$test_id} = 'Passed';
        }
    }
}

$sth->finish();
$handle->disconnect();

$pg_handle = DBI->connect(
    $POSTGRES_CONN_STRING,
    'postgres',
    undef
);

unless( $pg_handle )
{
    croak 'Failed to connect to database';
}

unless( $pg_handle->do( "DROP DATABASE \"$TEST_DATABASE\"" ) )
{
    croak( 'Failed to drop test database' );
}

$pg_handle->disconnect();

my $SUMMARY = <<"__TEXT__";
Result:
    Ran $test_count tests:
__TEXT__

my $failed_count = 0;
my $passed_count = 0;

foreach my $file ( sort { $a cmp $b } keys %$test_results )
{
    $SUMMARY .= "$file";
    my $file_passed_count = 0;
    my $file_failed_count = 0;

    foreach my $test_id( sort { $a <=> $b } keys %{$test_results->{$file}} )
    {
        my $result = $test_results->{$file}->{$test_id};

        if( $result =~ m/failed/i )
        {
            $file_failed_count++;
        }
        elsif( $result =~ m/passed/i )
        {
            $file_passed_count++;
        }
    }

    my $spacing_offset = $max_file_length - length( $file );
    my $SPACES = ' ' x $spacing_offset;
    $failed_count += $file_failed_count;
    $passed_count += $file_passed_count;
    my $total_file_tests = $file_failed_count + $file_passed_count;
    $SUMMARY .= "$SPACES $file_passed_count/$total_file_tests passed\n";
}

$SUMMARY .= " Total Failed: $failed_count\n Total Passed: $passed_count\n";

print $SUMMARY;
exit 0;
