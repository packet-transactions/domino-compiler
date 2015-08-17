/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "temp_file.hh"
#include "exception.hh"
#include "assert_exception.h"

using namespace std;

vector<char> to_mutable( const string & str )
{
    vector< char > ret;
    for ( const auto & ch : str ) {
        ret.push_back( ch );
    }
    ret.push_back( 0 ); /* null terminate */

    return ret;
}

UniqueFile::UniqueFile( const string & filename_template, const string & suffix )
    : mutable_temp_filename_( to_mutable( filename_template + "XXXXXX" + suffix ) ),
      fd_( SystemCall( "mkstemps", mkstemps( &mutable_temp_filename_[ 0 ], static_cast<int>(suffix.size()) ) ) ),
      moved_away_( false )
{
}

/* unlike UniqueFile, a TempFile is deleted when object destroyed */
TempFile::~TempFile()
{
    if ( moved_away_ ) { return; }

    try {
        SystemCall( "unlink " + name(), unlink( name().c_str() ) );
    } catch ( const exception & e ) {
        print_exception( e );
    }
}

void UniqueFile::write( const string & contents )
{
    assert_exception( not moved_away_ );

    fd_.write( contents );
}

UniqueFile::UniqueFile( UniqueFile && other )
    : mutable_temp_filename_( other.mutable_temp_filename_ ),
      fd_( move( other.fd_ ) ),
      moved_away_( false )
{
    other.moved_away_ = true;
}

string UniqueFile::name( void ) const
{
    assert_exception( mutable_temp_filename_.size() > 1 );
    return string( mutable_temp_filename_.begin(), mutable_temp_filename_.end() - 1 );
}
