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

#ifndef TMPFILE_H
#define TMPFILE_H
#include <string>
#include <boost/filesystem/path.hpp>

namespace isis
{
namespace util
{
/** Class to automatically create and handle a temporary file.
 * The file will be created by the constructor and deleted by the destructor.
 * If its not there anymore, a warning will be send.
 * This inherits from boost::filesystem::path and thus can be used as such.
 */
class TmpFile: public boost::filesystem::path
{
public:
	/** Create a temporary file.
	 * This generates a temporary filename using the given prefix and suffix.
	 * The file will also be created to prevent any following calls from generating the same filename.
	 * \param prefix string to be inserted between the path and the actual filename
	 * \param suffix string to be appended to the filename
	 */
	TmpFile( std::string prefix = "", std::string sufix = "" );
	///Will delete the temporary file if its still there.
	~TmpFile();
};
}
}
#endif // TMPFILE_H
