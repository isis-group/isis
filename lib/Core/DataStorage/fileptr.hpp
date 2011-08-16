//
//  fileptr.hpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef FILEPTR_HPP
#define FILEPTR_HPP

#define BOOST_FILESYSTEM_VERSION 2 //@todo remove as soon as we switch to boost 1.42
#include <boost/filesystem.hpp>
#include "typeptr.hpp"

namespace isis{
namespace data{

class FilePtr:public ValuePtr<uint8_t> {
	struct Closer{
		int file;
		size_t len;
		boost::filesystem::path filename;
		void operator()(void *p);
	};

	bool map(int file,size_t len,bool write,const boost::filesystem::path &filename);
	size_t checkSize(bool write,int file, const boost::filesystem::path &filename,size_t size=0);
public:
	/**
	 * Create a FilePtr mapping to the given file.
	 * if the write is true:
	 * - the file will be created if it does not exist
	 * - the file will be resized to len if it is shorter than len
	 *
	 * if write is false:
	 * - len will become the size of the file if its 0
	 * - if len is not 0 but less then the filesize, only len bytes of the file will be mapped
	 *
	 * creation will fail (good()!=true afterwards) if:
	 * - open/mmap fail on the given file for any reason
	 * - if file does not exist, write is true and len is 0
	 * - if write is false and len is greater then the length of the file of the file does not exist
	 *
	 * \param filename the file to map into memory
	 * \param len the length in bytes of the maped memory (automatically set if 0)
	 * \param write the file be opened for writing (writing to the mapped memory will write to the file)
	 */	
	FilePtr(const boost::filesystem::path &filename,size_t len=0,bool write=false);
};

}
}

#endif
