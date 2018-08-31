//
//  fileptr.hpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef FILEPTR_HPP
#define FILEPTR_HPP

#define BOOST_FILESYSTEM_VERSION 3 
#include <boost/filesystem.hpp>
#include "bytearray.hpp"
#include "valuearray.hpp"
#include "endianess.hpp"

#ifdef WIN32
#include <windows.h>
#define FILE_HANDLE HANDLE
#else
#define FILE_HANDLE int
#endif

namespace isis
{
namespace data
{
/**
 * Class to map files into memory.
 * This can be used read only, or for read/write.
 *
 * Writing to a FilePtr mapping a file read-only is valid. It will not change the mapped file.
 *
 * This is inherting from ValueArray. Thus this, and all ValueArray created from it will be managed.
 * The mapped file will automatically unmapped and closed after all pointers are deleted.
 */
class FilePtr: public ByteArray
{
	struct Closer {
		FILE_HANDLE file, mmaph;
		size_t len;
		boost::filesystem::path filename;
		bool write;
		void operator()( void *p );
	};
	bool map( FILE_HANDLE file, size_t len, bool write, const boost::filesystem::path &filename );

	size_t checkSize( bool write, FILE_HANDLE file, const boost::filesystem::path &filename, size_t size = 0 );
	bool m_good;
public:
	/// empty creator - result will not be usefull until filled
	FilePtr();
	/**
	 * Create a FilePtr, mapping the given file.
	 * if the write is true:
	 * - the file will be created if it does not exist
	 * - the file will be resized to len if it is shorter than len
	 * - a resize after this creation is NOT possible
	 *
	 * if write is false:
	 * - the length of the pointer will become the size of the file if len is 0 (so the whole file will be mapped)
	 * - if len is not 0 but less then the filesize, only len bytes of the file will be mapped
	 *
	 * creation will fail (good()!=true afterwards) if:
	 * - open/mmap fail on the given file for any reason
	 * - if file does not exist, write is true and len is 0
	 * - if write is false and len is greater then the length of the file or the file does not exist
	 *
	 * \param filename the file to map into memory
	 * \param len the requested length of the resulting ValueArray in bytes (automatically set if 0)
	 * \param write the file be opened for writing (writing to the mapped memory will write to the file, otherwise it will cause a copy-on-write)
	 */
	FilePtr( const boost::filesystem::path &filename, size_t len = 0, bool write = false );

	bool good();
	void release();
};

}
}

#endif
