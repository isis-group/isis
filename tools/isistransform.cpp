#include <isis/data/io_application.hpp>
#include <isis/data/io_factory.hpp>
#include <isis/math/transform.hpp>
#include <isis/adapter/itk/common.hpp>
#include <isis/adapter/itk/common.hxx>

#include <map>
#include <boost/assign.hpp>

using namespace isis;

struct TransformLog {static const char *name() {return "Transform";}; enum {use = _ENABLE_LOG};};
struct TransformDebug {static const char *name() {return "TransformDebug";}; enum {use = _ENABLE_DEBUG};};

class : public data::ChunkOp
{
public:
	data::dimensions dim;
	bool operator()( data::Chunk &ch, util::vector4<size_t> /*posInImage*/ ) {
		ch.flipAlong( dim );
		return true;
	}
} flifu;

bool swapProperties( data::Image &image, const unsigned short dim )
{
	const util::vector4<size_t> size = image.getSizeAsVector();
	std::vector<data::Chunk> chunks = image.copyChunksToVector( true );

	if( chunks.front().getRelevantDims() < 2 && dim >= chunks.front().getRelevantDims() ) {
		LOG( data::Runtime, error ) << "Your data is spliced at dimension " << chunks.front().getRelevantDims()
									<< " and you trying to flip at dimenstion " << dim << ". Sorry but this has not yet been impleneted.";
		return false;
	}

	std::vector<data::Chunk> buffer = chunks;
	std::vector<data::Chunk> swapped_chunks;

	for( util::ivector4::value_type one_above_dim = 0; one_above_dim < size[dim + 1]; one_above_dim++ ) {
		util::ivector4::value_type reverse_count = size[dim] - 1;

		for( util::ivector4::value_type count = 0; count < size[dim]; count++, reverse_count-- ) {
			static_cast<util::PropertyMap &>( chunks[count] ) = static_cast<util::PropertyMap &>( buffer[reverse_count] );
		}
	}

	std::list<data::Chunk> chunk_list( chunks.begin(), chunks.end() );
	image = data::Image( chunk_list );
	return true;
}


int main( int argc, char **argv )
{
	ENABLE_LOG( data::Runtime, util::DefaultMsgPrint, error );
	

	data::IOApplication app( "isistransform", true, true );
	app.parameters["swapdim"] = util::slist{"x","y","z", "t"};
	app.parameters["swapdim"].needed()=false;
	
	app.parameters["resample"]=util::ivector4{-1,-1,-1,-1};
	app.parameters["resample"].needed()=false;
	
	app.addLogging<TransformLog>();
	app.addLogging<TransformDebug>();
	
	app.init( argc, argv );
	
	util::slist swapdim_list=app.parameters["swapdim"];
	LOG_IF(swapdim_list.size()>4,TransformLog,warning)	<< "Ignoring all but 4 given parameters for swapdim";
	swapdim_list.resize(4);
	
	//go through every image
	for( data::Image & refImage :  app.images ) {
		
		if(app.parameters["swapdim"].isParsed()){
			int dim=0;
			for(std::string swap:swapdim_list){
				int idx=0;bool flip=false;
				if(swap[idx]=='-'){
					flip=true;
					idx++;
				} else if(swap[idx]=='+'){
					idx++;
				} 
				int target=std::tolower(swap[idx])-'x';
				if(swap[idx]=='t')target=3;
				if(target<0 || target>3){
					target=dim;
					LOG(TransformLog,warning) << "Ignorig unknown swapdim parameter " << swap;
					continue;
				}
				
				if( refImage.getChunkAt(0).getRelevantDims() > target ) {//dimension to flip is inside the chunks (so flip them)
					flifu.dim = static_cast<data::dimensions>( target );
					refImage.foreachChunk( flifu );
				} else { // otherwhise just flip the Chunks positions
					if( !swapProperties( refImage, target ) ) {
						return EXIT_FAILURE;
					}
				}

				LOG(TransformLog,info) << "swapping dim " << dim << " and " << target;
				refImage.swapDim(dim,target, app.feedback());
			}
		}
		if(app.parameters["resample"].isParsed()){
			util::vector4<size_t> oldsize=refImage.getSizeAsVector(),newsize;
			util::ivector4 reqsize=app.parameters["resample"];
			for(int i=0;i<4;i++){
				newsize[i]=(reqsize[i]!=-1)?
					reqsize[i]:oldsize[i];
			}
			refImage=itk4::resample(refImage,newsize);
		}
	}
	app.autowrite( app.images );
	return 0;
}

