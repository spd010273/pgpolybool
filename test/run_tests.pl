#!/usr/bin/perl

use warnings;
use strict;

use DBI;
use Carp;
use JSON;
use Readonly;
use English qw( -no_match_vars );

Readonly my $TEST_DIR             => 'sql';
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
    croak( "Test database '$TEST_DATABASE' already exists" );
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
unless( opendir( TESTDIR, $TEST_DIR ) )
{
    croak( "Failed to open test directory '$TEST_DIR': $OS_ERROR" );
}

my @files = readdir( TESTDIR );
closedir( TESTDIR );

my $test_results = {};

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

    my $test_fh;
    my $test_name = $file;
    $test_name =~ s/\.json$//;

    unless( open( $test_fh, '<:encoding(UTF-8)', "${TEST_DIR}/${file}" ) )
    {
        croak( "Failed to open '${TEST_DIR}/${file}': $OS_ERROR" );
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
        }
        else
        {

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
exit 0;
