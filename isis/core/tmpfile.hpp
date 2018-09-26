/*
    Copyright (C) 2010  reimer@cbs.mpg.de

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
#define BOOST_FILESYSTEM_VERSION 3 
#include <boost/filesystem/path.hpp>

namespace isis
{
namespace util
{
/** Class to automatically create and handle a temporary file.
 * This can e.g. be used when writing plugins for image formats to have a mock object.
 * The file will be created by the constructor and deleted by the destructor.
 * If its not there anymore, a warning will be send.
 * This inherits from boost::filesystem::path and thus can be used as such.
 */
class TmpFile: public boost::filesystem::path, boost::noncopyable
{
private:
	// dont do this
	TmpFile( TmpFile & );
	TmpFile &operator=( TmpFile & );
public:
	/** Create a temporary file.
	 * This generates a temporary filename using the given prefix and suffix.
	 * The file will also be created to prevent any following calls from generating the same filename.
	 * \param prefix string to be inserted between the path and the actual filename
	 * \param suffix string to be appended to the filename
	 * \note This uses boost::filesystem::unique_path and thus may block until sufficient entropy develops. (see http://www.boost.org/doc/libs/1_49_0/libs/filesystem/v3/doc/reference.html#unique_path)
	 */
	TmpFile( std::string prefix = "", std::string suffix = "" );
	///Will delete the temporary file if its still there.
	~TmpFile();
};
}
}
#endif // TMPFILE_H
