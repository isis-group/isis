//
//  fileptr.cpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "fileptr.hpp"

#include <boost/mpl/for_each.hpp>
#include "../CoreUtils/singletons.hpp"

namespace isis
{
namespace data
{

void FilePtr::Closer::operator()( void *p )
{
	LOG( Debug, info ) << "Unmapping and closing " << util::MSubject( filename ) << " it was mapped at " << p;
	file.close();
}

FilePtr::GeneratorMap::GeneratorMap()
{
	boost::mpl::for_each<util::_internal::types>( proc( this ) );
	assert( !empty() );
}


bool FilePtr::map( const boost::filesystem::path &filename )
{
	char* ptr = file.data();
	if( ptr == NULL ) {
		LOG( Debug, error ) << "Failed to map " << util::MSubject( filename ) << ", error was " << util::getLastSystemError();
		return false;
	} else {
		return true;
	}
}

FilePtr::FilePtr(){}

FilePtr::FilePtr( const boost::filesystem::path &filename, size_t len, bool write )
{
	boost::iostreams::mapped_file_params par(filename.native());
	if(write){
		par.flags=mapped_file::readwrite;
		par.new_file_size=len;
	} else
		par.flags=mapped_file::priv;
	
	try{
		file.open(par);
		if( file.is_open() && file.const_data()) {
			const Closer cl = {file, filename};
			static_cast<ValueArray<uint8_t>&>( *this ) = ValueArray<uint8_t>( reinterpret_cast<uint8_t * const>( file.data() ), file.size(), cl );
			LOG( Debug, info ) << "Mapped " << file.size() << " bytes of " << util::MSubject( filename ) << " at " << getRawAddress().get();
		}else{
			LOG( Runtime, error ) << "Failed to mmap " << util::MSubject(filename) << ", the error was: " << util::getLastSystemError();
			return;
		}
	} catch (std::ios_base::failure &e){
		LOG( Runtime, error ) << "Failed to mmap " << util::MSubject(filename) << ", the error was: " << e.what();
	}
}

bool FilePtr::good() {return file.const_data();}

void FilePtr::release()
{
	static_cast<std::shared_ptr<uint8_t>&>( *this ).reset();
}

ValueArrayReference FilePtr::atByID( short unsigned int ID, size_t offset, size_t len, bool swap_endianess )
{
	LOG_IF( static_cast<std::shared_ptr<uint8_t>&>( *this ).get() == 0, Debug, error )
			<< "There is no mapped data for this FilePtr - I'm very likely gonna crash soon ..";
	GeneratorMap &map = util::Singletons::get<GeneratorMap, 0>();
	assert( !map.empty() );
	const generator_type gen = map[ID];
	assert( gen );
	return gen( *this, offset, len, swap_endianess );
}


}
}
