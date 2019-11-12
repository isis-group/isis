#include <isis/core/chunk.hpp>
#include <isis/core/image.hpp>
#include <boost/timer.hpp>

using namespace isis;

template<typename T> void testChunk( )
{
	data::MemChunk<T> ch(200,100,50,25);
	
	std::cout << "dim-swapped " << data::ValueArray<T>::staticName() << " " << ch.getSizeAsVector() << "-Chunk to ";
	
	boost::timer timer;
	ch.swapDim(data::sliceDim,data::timeDim);
	std::cout << " to " << ch.getSizeAsVector() << " in " << timer.elapsed() << " seconds " << std::endl;
	
}

template<typename T> void testImage( )
{
	std::list<data::MemChunk<T> > chunks;
	for(int i=0; i<25;i++){
		data::MemChunk<T> ch( 200,100,50 );
		ch.setValueAs( "indexOrigin", util::fvector3( {0, 0, 0} ) );
		ch.setValueAs( "rowVec", util::fvector3( {1, 0} ) );
		ch.setValueAs( "columnVec", util::fvector3( {0, 1} ) );
		ch.setValueAs( "sliceVec", util::fvector3( {0, 0, 1} ) );
		ch.setValueAs( "voxelSize", util::fvector3( {1, 1, 1} ) );

		ch.setValueAs( "acquisitionNumber", ( uint32_t )i );
		ch.setValueAs( "sequenceNumber", ( uint16_t )0 );
		chunks.push_back(ch);
	}
	
	data::Image img( chunks );
	
	std::cout << "dim-swapped " << img.getMajorTypeName() << " " << img.getSizeAsVector() << "-Image to ";
	
	boost::timer timer;
	img.swapDim(data::sliceDim,data::timeDim);
	std::cout << " to " << img.getSizeAsVector() << " in " << timer.elapsed() << " seconds " << std::endl;
	
}

int main()
{
// 	testChunk< int8_t>();
// 	testChunk<int16_t>();
// 	testChunk<int32_t>();
// 	
// 	testChunk< uint8_t>();
// 	testChunk<uint16_t>();
// 	testChunk<uint32_t>();
// 	
// 	testChunk< float>();
// 	testChunk<double>();

	testImage< int8_t>();
	testImage<int16_t>();
	testImage<int32_t>();
	
	testImage< uint8_t>();
	testImage<uint16_t>();
	testImage<uint32_t>();
	
	testImage< float>();
	testImage<double>();
	
	return 0;
}
