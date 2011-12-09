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
#include <unistd.h>
#include <boost/mpl/for_each.hpp>
#include "../CoreUtils/singletons.hpp"

// we need that, because boost::mpl::for_each will instantiate all types - and this needs the output stream operations
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace isis
{
namespace data
{

void FilePtr::Closer::operator()( void *p )
{
	LOG( Debug, info ) << "Unmapping and closing " << util::MSubject( filename );

	if( munmap( p, len ) != 0 ) {
		LOG( Runtime, warning )
				<< "Unmapping of " << util::MSubject( filename )
				<< " failed, the error was: " << util::MSubject( strerror( errno ) );
	}

	if( ::close( file ) != 0 ) {
		LOG( Runtime, warning )
				<< "Closing of " << util::MSubject( filename )
				<< " failed, the error was: " << util::MSubject( strerror( errno ) );
	}
}

FilePtr::GeneratorMap::GeneratorMap()
{
	boost::mpl::for_each<util::_internal::types>( proc( this ) );
	assert( !empty() );
}



bool FilePtr::map( int file, size_t len, bool write, const boost::filesystem::path &filename )
{
	const int flags = write ? MAP_SHARED : MAP_PRIVATE;

	void *const ptr = mmap( 0, len, PROT_WRITE | PROT_READ, flags , file, 0 ); // yes we say PROT_WRITE here also if the file is opened ro - its for the mapping, not for the file

	if( ptr == MAP_FAILED ) {
		LOG( Debug, error ) << "Failed to map file, error was " << strerror( errno );
		return false;
	} else {
		const Closer cl = {file, len, filename};
		static_cast<ValuePtr<uint8_t>&>( *this ) = ValuePtr<uint8_t>( static_cast<uint8_t * const>( ptr ), len, cl );
		return true;
	}
}

size_t FilePtr::checkSize( bool write, int file, const boost::filesystem::path &filename, size_t size )
{
	const boost::uintmax_t currSize = boost::filesystem::file_size( filename );

	if( write ) { // if we're writing
		assert( size > 0 );

		if( size > currSize ) { // and the file is shorter than requested, resize it
			const int err = ftruncate( file, size ) ? errno : 0;

			if( err ) { // could not resize the file => fail
				LOG( Runtime, error )
						<< "Failed to resize " << util::MSubject( filename )
						<< " to the requested size " << size << ", the error was: " << util::MSubject( strerror( err ) );
				return 0; // fail
			} else
				return size; // ok now the file has the right size
		} else
			return size; // no resizing needed
	} else { // if we're reading
		if( size == 0 ) {
			if ( std::numeric_limits<size_t>::max() < currSize ) {
				LOG( Runtime, error ) << "Sorry cannot map files larger than " << std::numeric_limits<size_t>::max()
									  << " bytes on this platform";
				return 0;
			} else
				return currSize; // automatically select size of the file
		} else if( size <= currSize )
			return size; // keep the requested size (will fit into the file)
		else { // size will not fit into the file (and we cannot resize) => fail
			LOG( Runtime, error ) << "The requested size for readonly mapping of " << util::MSubject( filename )
								  << " is greater than the filesize (" << currSize << ").";
			return 0; // fail
		}
	}
}

FilePtr::FilePtr():m_good(false){}


FilePtr::FilePtr( const boost::filesystem::path &filename, size_t len, bool write ): m_good( false )
{
	const int oflag = write ?
					  O_CREAT | O_RDWR : //create file if its not there
					  O_RDONLY; //open file readonly
	const int file = open( filename.file_string().c_str(), oflag, S_IRUSR | S_IWUSR );

	if( file == -1 ) {
		LOG( Runtime, error ) << "Failed to open " << util::MSubject( filename )
							  << ", the error was: " << util::MSubject( strerror( errno ) );
		return;
	}

	const size_t map_size = checkSize( write, file, filename, len ); // get the mapping size

	if( map_size )
		m_good = map( file, map_size, write, filename ); //and do the mapping

	// from here on the pointer will be set if mapping succeded
}

bool FilePtr::good() {return m_good;}

void FilePtr::release()
{
	static_cast<boost::shared_ptr<uint8_t>&>( *this ).reset();
	m_good = false;
}

ValuePtrReference FilePtr::atByID( short unsigned int ID, size_t offset, size_t len )
{
	LOG_IF(static_cast<boost::shared_ptr<uint8_t>&>( *this ).get()==0,Debug,error)
		<< "There is no mapped data for this FilePtr - I'm very likely gonna crash soon ..";
	GeneratorMap &map = util::Singletons::get<GeneratorMap, 0>();
	assert( !map.empty() );
	const generator_type gen = map[ID];
	assert( gen );
	return gen( *this, offset, len );
}


}
}