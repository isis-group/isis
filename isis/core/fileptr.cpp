//
//  fileptr.cpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "fileptr.hpp"

#ifdef WIN32
#else
#include <sys/mman.h>
#endif

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "singletons.hpp"

#include <sys/time.h>
#include <sys/resource.h>

namespace isis
{
namespace data
{

void FilePtr::Closer::operator()( void *p )
{
	LOG( Debug, verbose_info ) << "Unmapping and closing " << util::MSubject( filename ) << " it was mapped at " << p;

	bool unmapped = false;
#ifdef WIN32
	unmapped = UnmapViewOfFile( p );
#else
	unmapped = !munmap( p, len );
#endif
	LOG_IF( !unmapped, Runtime, warning )
			<< "Unmapping of " << util::MSubject( filename )
			<< " failed, the error was: " << util::MSubject( util::getLastSystemError() );

#ifdef __APPLE__

	if( write && futimes( file, NULL ) != 0 ) {
		LOG( Runtime, warning )
				<< "Setting access time of " << util::MSubject( filename )
				<< " failed, the error was: " << util::MSubject( strerror( errno ) );
	}

#elif WIN32

	if( write ) {
		FILETIME ft;
		SYSTEMTIME st;

		GetSystemTime( &st );
		bool ok = SystemTimeToFileTime( &st, &ft ) && SetFileTime( file, NULL, ( LPFILETIME ) NULL, &ft );
		LOG_IF( !ok, Runtime, warning )
				<< "Setting access time of " << util::MSubject( filename.file_string() )
				<< " failed, the error was: " << util::MSubject( util::getLastSystemError() );
	}

#endif

#ifdef WIN32

	if( !( CloseHandle( mmaph ) && CloseHandle( file ) ) ) {
#else

	if( ::close( file ) != 0 ) {
#endif
		LOG( Runtime, warning )
				<< "Closing of " << util::MSubject( filename )
				<< " failed, the error was: " << util::MSubject( strerror( errno ) );
	}
}

bool FilePtr::map( FILE_HANDLE file, size_t len, bool write, const boost::filesystem::path &filename )
{
	void *ptr = NULL;
	FILE_HANDLE mmaph = 0;
	rlimit rlim;
	getrlimit(RLIMIT_DATA,&rlim);
	
	if(rlim.rlim_cur<len){
		if(rlim.rlim_max>len){
			rlim.rlim_cur=len;
			setrlimit(RLIMIT_NOFILE, &rlim);
		} else {
			LOG(Runtime,warning) << "Can't increase the limit for for mapped file size to " << len << ", this will crash..";
		}
	}

#ifdef WIN32 //mmap is broken on windows - workaround stolen from http://crupp.de/2007/11/14/howto-port-unix-mmap-to-win32/
	mmaph = CreateFileMapping( file, 0, write ? PAGE_READWRITE : PAGE_WRITECOPY, 0, 0, NULL );

	if( mmaph ) {
		ptr = MapViewOfFile( mmaph, write ? FILE_MAP_WRITE : FILE_MAP_COPY, 0, 0, 0 );
	}

#else
	ptr = mmap( 0, len, PROT_WRITE | PROT_READ, write ? MAP_SHARED : MAP_PRIVATE , file, 0 ); // yes we say PROT_WRITE here also if the file is opened ro - its for the mapping, not for the file
#endif

	if( ptr == nullptr || ptr == reinterpret_cast<void*>(-1) ) {
		switch(errno){
			case ENOMEM:
				LOG( Runtime, error ) << "Excedet limit of mapping size or amount of mapped files";break;
			default:
				LOG( Runtime, error ) << "Failed to map " << util::MSubject( filename ) << ", error was " << util::getLastSystemError();
		}
		return false;
	} else {
		const Closer cl = {file, mmaph, len, filename, write};
		writing = write;
		static_cast<ByteArray&>( *this ) = ValueArray<uint8_t>( static_cast<uint8_t * const>( ptr ), len, cl );
		return true;
	}
}

size_t FilePtr::checkSize( bool write, FILE_HANDLE file, const boost::filesystem::path &filename, size_t size )
{
	const boost::uintmax_t currSize = boost::filesystem::file_size( filename );
	if ( std::numeric_limits<size_t>::max() < currSize ) {
		LOG( Runtime, error )
			<< "Sorry cannot map files larger than " << std::numeric_limits<size_t>::max()
			<< " bytes on this platform";
		return 0;
	}

	if( write ) { // if we're writing
		assert( size > 0 );

		if( size > currSize ) { // and the file is shorter than requested, resize it
			boost::system::error_code ec;
			boost::filesystem::resize_file(filename,size,ec);
			if(ec){
				 LOG( Runtime, error )
					<< "Failed to resize " << util::MSubject( filename )
					<< " to the requested size " << size << ", the error was: " << util::MSubject( ec.message() );
					return 0; // fail
			}
		}
		return boost::filesystem::file_size(filename);
	} else { // if we're reading
		if( size == 0 )
			return currSize; // automatically select size of the file
		else if( size <= currSize )
			return size; // keep the requested size (will fit into the file)
		else { // size will not fit into the file (and we cannot resize) => fail
			LOG( Runtime, error ) << "The requested size for readonly mapping of " << util::MSubject( filename )
								  << " is greater than the filesize (" << currSize << ").";
			return 0; // fail
		}
	}
}

FilePtr::FilePtr(): m_good( false ) {}


FilePtr::FilePtr( const boost::filesystem::path &filename, size_t len, bool write ): m_good( false )
{
#ifdef WIN32
	const FILE_HANDLE invalid = INVALID_HANDLE_VALUE;
	const int oflag = write ?
					  GENERIC_READ | GENERIC_WRITE :
					  GENERIC_READ; //open file readonly
	const FILE_HANDLE file =
		CreateFile( filename.file_string().c_str(), oflag, write ? 0 : FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
#else
	const FILE_HANDLE invalid = -1;
	const int oflag = write ?
					  O_CREAT | O_RDWR : //create file if its not there
					  O_RDONLY; //open file readonly
	const FILE_HANDLE file =
		open( filename.native().c_str(), oflag, 0666 );
#endif

	if( file == invalid ) {
		LOG( Runtime, error ) << "Failed to open " << util::MSubject(filename) << ", the error was: " << util::getLastSystemError();
		return;
	}

	const size_t map_size = checkSize( write, file, filename, len ); // get the mapping size

	if( map_size ) {
		m_good = map( file, map_size, write, filename ); //and do the mapping
		LOG( Debug, verbose_info ) << "Mapped " << map_size << " bytes of " << util::MSubject( filename ) << " at " << getRawAddress().get();
	}

	// from here on the pointer will be set if mapping succeded
}

bool FilePtr::good() {return m_good;}

void FilePtr::release()
{
	static_cast<std::shared_ptr<uint8_t>&>( *this ).reset();
	m_good = false;
}


}
}
