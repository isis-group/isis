/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <stdio.h>
#include <fstream>
#define BOOST_FILESYSTEM_VERSION 3 
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include "tmpfile.hpp"
#include "message.hpp"
#include "common.hpp"

namespace isis
{
namespace util
{

using namespace boost::filesystem;
  
TmpFile::TmpFile( std::string prefix, std::string sufix ): 
  path(temp_directory_path() / unique_path(prefix+"%%%%-%%%%-%%%%-%%%%"+sufix))
{
	LOG( Debug, info ) << "Creating temporary file " << native();
	ofstream( *this ).exceptions( std::ios::failbit | std::ios::badbit );
}

TmpFile::~TmpFile()
{
	if ( boost::filesystem::exists( *this ) ) {
		boost::filesystem::remove( *this );
		LOG( Debug, verbose_info ) << "Removing temporary " << native();
	} else {
		LOG( Debug, warning ) << "Temporary file " << native() << " does not exist, won't delete it";
	}
}
}
}
