#include "DataStorage/image.hpp"
#include <boost/timer.hpp>

using namespace isis;

const size_t chunk_size = 120;

template<typename TYPE>
void check( const data::Chunk &chunk, const TYPE &value )
{
	class DoCheck: public data::VoxelOp<TYPE>
	{
		const TYPE &value;
	public:
		bool operator()( TYPE &vox, const util::vector4<size_t> & ) {return vox == value;}
		DoCheck( const TYPE &_value ): value( _value ) {}
	} doCheck( value );

	if( const_cast<data::Chunk &>( chunk ).foreachVoxel( doCheck ) == 0 ) {
		std::cout << "check ok!" << std::endl;
	} else {
		std::cout << "check failed!" << std::endl;
	}
}

int main()
{
	typedef uint8_t TYPE;

	boost::timer timer;

	data::MemChunk<TYPE> big_chunk( chunk_size, chunk_size, chunk_size, chunk_size );
	const util::ivector4 size = big_chunk.getSizeAsVector();
	std::cout << "Chunk size is: " << size << std::endl;
	TYPE *ptr = &big_chunk.voxel<TYPE>( 0 );
	size_t volume = big_chunk.getVolume();

	timer.restart();

	for( size_t i = 0; i < volume; i++ ) {
		ptr[i] = 1;
	}

	std::cout << "Shortest iteration time is " << timer.elapsed() << " seconds." << std::endl;
	check<TYPE>( big_chunk, 1 );

	timer.restart();

	for( util::ivector4::value_type t = 0; t < size[data::timeDim]; t++ ) {
		for( util::ivector4::value_type s = 0; s < size[data::sliceDim]; s++ ) {
			for( util::ivector4::value_type c = 0; c < size[data::columnDim]; c++ ) {
				for( util::ivector4::value_type r = 0; r < size[data::rowDim]; r++ ) {
					TYPE &ref = big_chunk.voxel<TYPE>( r, c, s, t );
					ref = 2;
				}
			}
		}
	}

	std::cout << "Needed " << timer.elapsed() << " seconds with simple voxel function" << std::endl;
	check<TYPE>( big_chunk, 2 );

	timer.restart();

	for( util::ivector4::value_type t = 0; t < size[data::timeDim]; t++ ) {
		for( util::ivector4::value_type s = 0; s < size[data::sliceDim]; s++ ) {
			for( util::ivector4::value_type c = 0; c < size[data::columnDim]; c++ ) {
				for( util::ivector4::value_type r = 0; r < size[data::rowDim]; r++ ) {
					const size_t coords[] = { r, c, s, t };
					ptr[big_chunk.getLinearIndex( coords )] = 3;
				}
			}
		}
	}

	std::cout << "Needed " << timer.elapsed() << " seconds with explicit call of \"getLinearIndex()\"." << std::endl;
	check<TYPE>( big_chunk, 3 );

	timer.restart();
	size_t counter = 0;

	for( util::ivector4::value_type t = 0; t < size[data::timeDim]; t++ ) {
		for( util::ivector4::value_type s = 0; s < size[data::sliceDim]; s++ ) {
			for( util::ivector4::value_type c = 0; c < size[data::columnDim]; c++ ) {
				for( util::ivector4::value_type r = 0; r < size[data::rowDim]; r++ ) {
					const size_t lin_index = r +
											 size[data::rowDim] * c +
											 size[data::columnDim] * size[data::rowDim] * s  +
											 size[data::columnDim] * size[data::rowDim] * size[data::sliceDim] * t;
					ptr[lin_index] = 4;
					counter++;
				}
			}
		}
	}

	if( counter = big_chunk.getVolume() ) std::cout << "counter == volume!" << std::endl;

	std::cout << "Needed " << timer.elapsed() << " seconds to iterator with own \"getLinearIndex\" function." << std::endl;
	check<TYPE>( big_chunk, 4 );

	return 0;
}