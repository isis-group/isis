#include <DataStorage/io_application.hpp>
#include <DataStorage/io_factory.hpp>
#include <muParser/muParser.h>


using namespace isis;

class VoxelOp : public data::Chunk::VoxelOp<float>{
	mu::Parser parser;
	double voxBuff;
public:
	VoxelOp(std::string expr){
		parser.SetExpr(expr);
		parser.DefineVar(std::string("vox"),&voxBuff);
	}
	bool operator()(float& vox, const isis::util::FixedVector< size_t,4 >& pos){
		voxBuff=vox; //using parser.DefineVar every time would slow down the evaluation
		vox=parser.Eval();
	}

};

int main( int argc, char **argv )
{
	data::IOApplication app( "isis data converter", true, true );
	app.parameters["voxelop"]=std::string("vox");
	app.parameters["voxelop"].setDescription("a term to evaluate the new value of each voxel");
	app.init( argc, argv, true ); // will exit if there is a problem


	std::list<data::Image> out;
	std::string op=app.parameters["voxelop"];

	VoxelOp vop(op);

	BOOST_FOREACH(data::TypedImage<float> img,app.images){
		std::cout << "Computing vox=(" << op << ") for each voxel of the " << img.getSizeAsString() << "-Image" << std::endl;
		img.foreachVoxel<float>(vop);
		out.push_back(img);
	}

	app.autowrite( out );
	return EXIT_SUCCESS;
}
