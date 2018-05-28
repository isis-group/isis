#include <isis/data/io_application.hpp>
#include <isis/data/io_factory.hpp>
#include <isis/math/transform.hpp>
#include <isis/util/common.hpp>
#include <regex>
#include <cctype>

#include <isis/adapter/itk4/common.hpp>

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

int getDimFromStr(std::string s){
	int ret;
	if(std::tolower(s[0])=='t')
		ret=data::timeDim;
	else
		ret=std::tolower(s[0])-'x';
	
	return s[1]=='-'? -ret:ret;
}

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

void flipDim(data::Image &refImage, int dim){
	if(refImage.getChunkAt(0).getRelevantDims() > dim ) {//dimension to flip is inside the chunks (so flip them)
		LOG(TransformLog,notice) << "flipping voxels along dim " << std::string(1,'x'+dim)  << " in " << refImage.identify();
		flifu.dim = static_cast<data::dimensions>( dim );
		refImage.foreachChunk( flifu );
	} else { // otherwhise just flip the Chunks positions
		LOG(TransformLog,notice) << "flipping chunk order along dim " << std::string(1,'x'+dim)  << " in " << refImage.identify();
		if( !swapProperties( refImage, dim ) ) {
			exit(EXIT_FAILURE);
		}
	}
}

int main( int argc, char **argv )
{
	ENABLE_LOG( data::Runtime, util::DefaultMsgPrint, error );
	

	data::IOApplication app( "isistransform", true, true );
	app.parameters["swapdim"] = util::slist{};
	app.parameters["swapdim"].needed()=false;

	app.parameters["resample"]=util::ivector4{-1,-1,-1,-1};
	app.parameters["resample"].needed()=false;

	app.parameters["rotate"]=util::slist{"x","y", "90"};
	app.parameters["rotate"].needed()=false;

	app.parameters["translate"]=util::fvector3();
	app.parameters["translate"].needed()=false;
	
	app.parameters["pix_center"]=false;
	app.parameters["pix_center"].needed()=false;
	
	app.addLogging<TransformLog>("");
	app.addLogging<TransformDebug>("");
	
	app.addLogging<ITKLog>("ITK");
	app.addLogging<ITKDebug>("ITK");
	
	app.init( argc, argv );
	
	if(!(app.parameters["swapdim"].isParsed()||app.parameters["resample"].isParsed()||app.parameters["translate"].isParsed()||app.parameters["rotate"].isParsed())){
		LOG(TransformLog,error) << "No transformation requested, exiting..";
		LOG(TransformLog,notice) << "have at least one of " 
			<< util::MSubject("-swapdim") << ", " 
			<< util::MSubject("-translate") << ", "
			<< util::MSubject("-resample")  << " or " 
			<< util::MSubject("-rotate");
		exit(-1);
	}
	
	float rotate_angle=0;
	std::pair<int,int> rotate_plane;
	if(app.parameters["rotate"].isParsed()){
		util::slist rotate_list=app.parameters["rotate"];
		LOG_IF(rotate_list.size()>3,TransformLog,warning)	<< "Ignoring all but 3 given parameters for rotate";
		rotate_list.resize(3);
		try{
			rotate_angle = std::stof(rotate_list.back());
			rotate_list.pop_back();
			
			rotate_plane.first =std::tolower(rotate_list.front().front())-'x';
			rotate_plane.second=std::tolower(rotate_list.back().front())-'x';
			
			if(rotate_plane.first<0 || rotate_plane.first>2){
				LOG(TransformLog,error) << "first parameter for rotate must be x,y or z (was " << rotate_list.front() << ")";
				throw(std::out_of_range(rotate_list.front()));
			}
			if(rotate_plane.second<0 || rotate_plane.second>2){
				LOG(TransformLog,error) << "second parameter for rotate must be x,y or z (was " << rotate_list.front() << ")";
				throw(std::out_of_range(rotate_list.front()));
			}
			if(rotate_plane.first == rotate_plane.second){
				LOG(TransformLog,error) << "first and second parameter for rotate must not be equal";
				throw(std::logic_error("first and second parameter for rotate must not be equal"));
			}
		}
		catch(std::logic_error &err){
			LOG(TransformLog,error) << "failed to parse rotate parameters (" << err.what() << ")";
			exit(EXIT_FAILURE);
		}
	}

	std::list<std::pair<std::string,std::string>> swapper;
	for(std::string cmd:app.parameters["swapdim"].as<util::slist>()){
		static const std::regex cmd_regex("([xyzt]-?)(:([xyzt]-?))?",std::regex_constants::icase);
		std::smatch results;
		
		if(std::regex_match(cmd,results,cmd_regex)){
			swapper.push_back(std::make_pair(results[1],""));
			if(results.length(3)>0)
				swapper.back().second=results[3];
		} else {
			LOG(TransformLog,error) << "ignoring invalid swapdim parameter " << cmd;
		}
	}

	//go through every image
	for( data::Image & refImage :  app.images ) {
		for(auto swap:swapper){ // swapdim
			if(swap.first[1]=='-'){
				swap.first.pop_back();
				flipDim(refImage,getDimFromStr(swap.first));
			}

			if(swap.second[1]=='-'){
				swap.second.pop_back();
				flipDim(refImage,getDimFromStr(swap.second));
			}
			
			if(!swap.second.empty() && swap.first != swap.second){
				LOG(TransformLog,notice) << "swapping dim " << swap.first << " and " << swap.second << " in " << refImage.identify();
				refImage.swapDim(getDimFromStr(swap.first),getDimFromStr(swap.second), app.feedback());
			}
		}
		if(app.parameters["translate"].isParsed()){
			refImage=itk4::translate(refImage,app.parameters["translate"]);
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
		if(rotate_angle){
			refImage=itk4::rotate(refImage,rotate_plane,rotate_angle, app.parameters["pix_center"]);
		}
	}
	app.autowrite( app.images );
	return 0;
}

