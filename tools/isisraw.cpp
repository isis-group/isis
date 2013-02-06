#include <DataStorage/io_factory.hpp>
#include <DataStorage/io_application.hpp>
#include <DataStorage/fileptr.hpp>


struct RawLog {static const char *name() {return "Raw";}; enum {use = _ENABLE_LOG};};
struct RawDebug {static const char *name() {return "RawDebug";}; enum {use = _ENABLE_DEBUG};};

using namespace isis;

// add a faked file format class, so we can generate unique filenames
class FakedRawFormat: public image_io::FileFormat
{
	std::string getName()const {return "";};
	int load( std::list< data::Chunk >& /*chunks*/, const std::string & /*filename*/, const util::istring & /*dialect*/, boost::shared_ptr<util::ProgressFeedback> /*progress*/ ) throw( std::runtime_error & ) {return 0;}
	util::istring suffixes( io_modes /*modes = both*/ ) const {return "";}
	void write( const data::Image & /*image*/, const std::string & /*filename*/, const util::istring & /*dialect*/, boost::shared_ptr<util::ProgressFeedback> /*progress*/ ) throw( std::runtime_error & ) {}
	std::pair< std::string, std::string > makeBasename( const std::string &filename ) {
		return std::make_pair( filename, std::string( "" ) );
	}
};

int main( int argc, char *argv[] )
{
	data::IOApplication app( "isis raw reader/writer", false, false );
	// ********* Parameters *********

	app.addInput( app.parameters );
	app.addOutput( app.parameters );

	app.parameters["byteswap"] = false;
	app.parameters["byteswap"].needed() = false;
	app.parameters["byteswap"].setDescription( "swap byte endianess when reading raw data (ignored when read_repn is not given)" );

	app.parameters["offset"] = uint64_t();
	app.parameters["offset"].needed() = false;
	app.parameters["offset"].setDescription( "offset in bytes where to start inside the raw file" );

	app.parameters["read_repn"] = app.parameters["repn"]; // steal the supported data types from repn
	app.parameters["read_repn"].needed() = false;
	app.parameters["read_repn"].setDescription( "data type of the raw file (if given, mode for reading raw is assumed)" );

	app.parameters["rawdims"] = util::ivector4( 0, 1, 1, 1 );
	app.parameters["rawdims"].needed() = false;
	app.parameters["rawdims"].setDescription( "the dimensions of the raw image, at least number of columns must be given (ignored when read_repn is not given)" );

	app.parameters["voxel"] = util::fvector3( 1, 1, 1 );
	app.parameters["voxel"].needed() = false;
	app.parameters["voxel"].setDescription( "the size of the voxels in each direction" );

	app.parameters["origin"] = util::fvector3( );
	app.parameters["origin"].needed() = false;
	app.parameters["origin"].setDescription( "the position of the first voxel in isis space" );

	app.addExample( "-in my_file.nii -out /tmp/raw.file -repn u8bit", "Write the image data of a nifti file in a u8bit raw file" );
	app.addExample( "-in raw.file -read_repn s16bit -out new_image.nii -rawdims 384 384 12 -offset 500", "Read 384*384*12 s16bit blocks from a raw file skipping 500 bytes and store them as a nifti image." );

	app.init( argc, argv, true ); // if there is a problem, we just get no images and exit cleanly

	const uint64_t offset = app.parameters["offset"];

	if( app.parameters["read_repn"].isSet() ) { // reading raw
		util::slist infiles = app.parameters["in"];
		LOG_IF( infiles.size() > 1, RawLog, warning ) << "Cannot read multiple raw files at once, will only read " << infiles.front();
		data::FilePtr src( infiles.front() );
		const unsigned short rrepn = app.parameters["read_repn"].as<util::Selection>();
		util::ivector4 dims = app.parameters["rawdims"];
		data::ValueArrayReference dat;

		if( dims.product() == 0 ) {
			data::ValueArrayReference dat = src.atByID( rrepn, offset, 0, app.parameters["byteswap"] );

			const size_t sidelength = sqrt( dat->getLength() );

			if( sidelength *sidelength == dat->getLength() ) {
				LOG( RawLog, warning ) << "No or invalid dimensions given in rawdims, assuming squared 2D image";
				dims.fill( 1 );
				dims[data::rowDim] = dims[data::columnDim] = sidelength;
			} else {
				LOG( RawLog, error ) << "No or invalid dimensions given in rawdims and datasize does not fit a squared 2D image, aborting...";
				exit( -1 );
			}
		} else {
			dat = src.atByID( rrepn, offset, dims.product(), app.parameters["byteswap"] );
		}

		LOG( RawLog, notice ) << "Reading " <<  dat->getLength()*dat->bytesPerElem() / ( 1024.*1024. ) << " MBytes from " << infiles.front();

		data::Chunk ch( dat, dims[data::rowDim], dims[data::columnDim], dims[data::sliceDim], dims[data::timeDim], true );
		ch.setPropertyAs<util::fvector3>( "indexOrigin", app.parameters["origin"] );
		ch.setPropertyAs<util::fvector3>( "voxelSize", app.parameters["voxel"] );

		app.autowrite( data::Image( ch ), true );
	} else { // writing raw
		app.autoload( true ); //load "normal" images

		const std::list< std::string > fnames = FakedRawFormat().makeUniqueFilenames( app.images, app.parameters["out"] );
		std::list< std::string >::const_iterator iOut = fnames.begin();
		const util::Selection wrepn = app.parameters["repn"];

		BOOST_FOREACH( const data::Image & img, app.images ) {
			const unsigned short sRepn = ( int )wrepn ? : img.getMajorTypeID(); // get repn eigther from the parameter, or from the image
			size_t repnsize = data::ValueArrayBase::createByID( sRepn, 1 )->bytesPerElem(); //create a dummy ValueArray to determine the elementsize of the requested repn
			const size_t imgsize = img.getVolume() * repnsize;
			const std::string filename = *( iOut++ );

			LOG( RawLog, notice ) << "Writing " << imgsize / ( 1024.*1024. ) << " MBytes from an " << img.getSizeAsString() << "-image as " << util::getTypeMap()[sRepn] << " to " << filename;
			data::FilePtr f( filename, imgsize + offset, true );
			data::ValueArrayReference dat = f.atByID( sRepn, offset ); //if repn is unset use the type of the image
			img.copyToValueArray( *dat );
		}
	}

	return 0;
}