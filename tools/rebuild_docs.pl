#!/usr/bin/perl

use strict;
use warnings;

use Carp;
use Readonly;
use English qw( -no_match_var );

Readonly my $DOC_DIR => '../docs/';

# Start by enumerating versions in this branch and clearing out
# old compiled documentation
unless( opendir( VERSIONS, $DOC_DIR ) )
{
    croak( "Could not open docs directory '$DOC_DIR': $OS_ERROR" );
}

my @files = readdir( VERSIONS );
closedir( VERSIONS );

my @versions;

foreach my $file( @files )
{
    if( $file =~ m/^\.$/ || $file =~ m/^\.\.$/ )
    {
        next;
    }

    if( $file =~ m/\.md$/i )
    {
        next;
    }

    if( -d "${DOC_DIR}${file}" && $file =~ m/^V\d{1,2}\.\d{1,2}$/m )
    {
        push( @versions, $file );
    }
}

# Look in the version directory
foreach my $version( sort { $a cmp $b } @versions )
{
    my $target_version_file = "${DOC_DIR}${version}.md";

    # Remove the version's documentation - we'll be overwriting this
    if( -e $target_version_file )
    {
        unlink( $target_version_file );
    }

    # Open the version's docs for writing
    my $target_fh;
    unless( open( $target_fh, '>:encoding(UTF-8)', $target_version_file ) )
    {
        croak( "Failed to open '$target_version_file' for write: $OS_ERROR" );
    }

    print "Building '${DOC_DIR}${version}'...\n";

    unless( opendir( SECTIONS, "${DOC_DIR}${version}" ) )
    {
        croak( "Could not open directory '${DOC_DIR}${version}'" );
    }

    my @sections = readdir( SECTIONS );
    closedir( SECTIONS );

    if( -e "${DOC_DIR}${version}/__header.md" )
    {
        unless( open( VERSION_HEADER, '<:encoding(UTF-8)', "${DOC_DIR}${version}/__header.md" ) )
        {
            croak( "Failed to open version header file '${DOC_DIR}${version}/__header.md': $OS_ERROR" );
        }

        while( my $line = <VERSION_HEADER> )
        {
            print $target_fh $line;
        }

        close( VERSION_HEADER );
        print $target_fh "\n\n";
    }

    # Enumerate the sections under the current version
    foreach my $section( sort { $a cmp $b } @sections )
    {
        if( $section =~ m/^\.$/ || $section =~ m/^\.\.$/ )
        {
            next;
        }

        my $file = "${DOC_DIR}${version}/$section";

        if( -d $file )
        {
            # We've found a section - parse the section header and append that to target_fh
            #  then parse the other .md files and append those

            unless( opendir( SECTION_FILES, $file ) )
            {
                croak( "Failed to open section directory '$file': $OS_ERROR" );
            }

            my @section_files = readdir( SECTION_FILES );
            closedir( SECTION_FILES );

            my $header_file = "$file/__header.md";

            if( -e $header_file )
            {
                unless( open( HEADER_FH, $header_file ) )
                {
                    croak( "Failed to open section header '$header_file': $OS_ERROR" );
                }

                while( my $line = <HEADER_FH> )
                {
                    print $target_fh $line;
                }

                print $target_fh "\n\n";
                close( HEADER_FH );
            }

            # Enumerate the documentation under each section
            foreach my $section_file ( sort { $a cmp $b } @section_files )
            {
                if( $section_file =~ m/^\.$/ || $section_file =~ m/^\.\.$/ )
                {
                    next;
                }

                if( $section_file =~ m/^__header\.md$/ )
                {
                    next;
                }

                if( $section_file =~ m/\.md$/i )
                {
                    my $doc_file = "${file}/${section_file}";

                    unless( open( DOC_FH, $doc_file ) )
                    {
                        croak( "Failed to open documentation file '$doc_file': $OS_ERROR" );
                    }

                    while( my $line = <DOC_FH> )
                    {
                        print $target_fh $line;
                    }

                    print $target_fh "\n\n";
                    close( DOC_FH );
                }
            } # section files
        }
    } # sections

    close( $target_fh );
} # versions
