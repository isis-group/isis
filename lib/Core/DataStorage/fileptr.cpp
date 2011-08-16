//
//  fileptr.cpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "fileptr.hpp"
#include <sys/mman.h>
#include <fcntl.h>

namespace isis {
namespace data {

void FilePtr::Closer::operator()(void *p){
	LOG(Debug,info) << "Unmapping and closing " << util::MSubject(filename);
	if(munmap(p, len)!=0)
		LOG(Runtime,warning) 
		<< "Unmapping of " << util::MSubject(filename) 
		<< " failed, the error was: " << util::MSubject(strerror(errno));
	if(close(file)!=0)
		LOG(Runtime,warning) 
		<< "Closing of " << util::MSubject(filename) 
		<< " failed, the error was: " << util::MSubject(strerror(errno));
}
	
bool FilePtr::map(int file, size_t len, bool write,const boost::filesystem::path &filename){
	void *const ptr=mmap(0, len, write ? PROT_WRITE:PROT_READ, write ? MAP_SHARED:MAP_PRIVATE, file, 0);
	if(ptr==MAP_FAILED){
		LOG(Debug, error) << "Failed to map file, error was " << strerror(errno);
		return false;
	} else {
		const Closer cl={file,len,filename};
		static_cast<ValuePtr<uint8_t>&>(*this)=ValuePtr<uint8_t>(static_cast<uint8_t *const>(ptr), len, cl);
		return true;
	}
}
	
size_t FilePtr::checkSize(bool write,int file,const boost::filesystem::path &filename,size_t size){
	const size_t currSize=boost::filesystem::file_size(filename);
	if(write){ // if we're writing
		assert(size>0);
		if(size>currSize){ // and the file is shorter than requested, resize it
#ifdef HAVE_FALLOCATE
			const int err = fallocate( file, 0, 0, size ); //fast preallocation using features of some linux-filesystems
#elif HAVE_POSIX_FALLOCATE
			const int err = posix_fallocate( file, 0, size ); // slower posix compatible version
#else
			const int err = ( lseek( file, size - 1, SEEK_SET ) == off_t( size - 1 ) && ::write( file, " ", 1 ) ) ? 0 : errno; //workaround in case there is no fallocate
#endif
			if(err){ // could not reize the file => fail
				LOG(Runtime,error) 
				<< "Failed to resize " << util::MSubject(filename) 
				<< " to the requested size " << size << ", the error was: " << util::MSubject(strerror(err));
				return 0; // fail
			} else 
				return size; // ok now the file has the right size
		} else
			return size; // no resizing needed
	} else {
		if(size==0)
			return currSize; // autmotacially select size of the file
		else if(size<=currSize)
			return size; // keep the requsted size (will fit into the file)
		else{ // size will not fit into the file (and we cannot resize) => fail
			LOG(Runtime,error) << "The requested size for readonly mapping of " << util::MSubject(filename) 
			<< " is greater than the filesize (" << currSize << ").";
			return 0; // fail
		}
	}
}

	
FilePtr::FilePtr(const boost::filesystem::path &filename,size_t len,bool write){
	const int oflag = write ? 
		O_CREAT|O_RDWR|O_EXLOCK: //create file if its not there, get exculsive lock on it
		O_RDONLY|O_SHLOCK; //open file readonly, get shared lock
	const int file=open(filename.file_string().c_str(),oflag,S_IRUSR | S_IWUSR);
	if(file==-1){
		LOG(Runtime,error) << "Failed to open " << util::MSubject(filename) 
			<< ", the error was: " << util::MSubject(strerror(errno));
		return;
	}
	const size_t map_size=checkSize(write, file, filename,len); // get the mapping size
	if(map_size)
		map(file, map_size, write, filename); //and do the mapping
	// from here on the pointer will be set if mapping succeded
}
	
}
}