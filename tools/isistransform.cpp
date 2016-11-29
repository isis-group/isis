#include <isis/data/io_application.hpp>
#include <isis/data/io_factory.hpp>
#include <isis/math/transform.hpp>

#include <map>
#include <boost/assign.hpp>

using namespace isis;

struct TransformLog {static const char *name() {return "Transform";}; enum {use = _ENABLE_LOG};};
struct TransformDebug {static const char *name() {return "TransformDebug";}; enum {use = _ENABLE_DEBUG};};


int main( int argc, char **argv )
{
	ENABLE_LOG( data::Runtime, util::DefaultMsgPrint, error );
	
	util::Selection face("sagittal,coronal,axial");
	
	util::Selection x("x,y,z,x-,y-,z-","x"),y("x,y,z,x-,y-,z-","y"),z("x,y,z,x-,y-,z-","z");
	
	util::Selection mode( "image,space,both","both" );

	data::IOApplication app( "isistransform", true, true );
	app.parameters["face"] = face;
	app.parameters["face"].needed() = false;
	
	app.parameters["x"]=x;
	app.parameters["x"].needed()=false;
	app.parameters["y"]=y;
	app.parameters["y"].needed()=false;
	app.parameters["z"]=z;
	app.parameters["z"].needed()=false;

	app.parameters["mode"] = mode;
	app.parameters["mode"].needed() = false;
	app.parameters["mode"].setDescription( "What should be transformed" );
	
	app.addLogging<TransformLog>();
	app.addLogging<TransformDebug>();
	
	app.init( argc, argv );
	
	face=app.parameters["face"];
	util::Selection dims[]={app.parameters["x"],app.parameters["y"],app.parameters["z"]};

	//go through every image
	for( data::Image & refImage :  app.images ) {
		for(int i=0;i<3;i++){
			unsigned short dim=dims[i]-1;
			if(dim>3){
				LOG(TransformLog,info) << "flipping dim " << dim%3;
				dim%=3;
			}
			if(dim!= i){
				LOG(TransformLog,info) << "swapping dim " << i << " and " << dim;
				refImage.swapDim(i,dim, app.feedback());
				std::swap(dims[i],dims[dim]);
			}
		}
	}
	app.autowrite( app.images );
	return 0;
}

