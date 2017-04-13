#include <isis/data/io_interface.h>
#include <png.h>
#include <stdio.h>

namespace isis
{
namespace image_io
{

class ImageFormat_png: public FileFormat
{
protected:
	util::istring suffixes( io_modes /*modes = both */ )const {return ".png";}
	struct Reader {
		virtual data::Chunk operator()( png_structp png_ptr, png_infop info_ptr )const = 0;
		virtual ~Reader() {}
	};
	template<typename TYPE> struct GenericReader: Reader {
		data::Chunk operator()( png_structp png_ptr, png_infop info_ptr )const {
			const png_uint_32 width = png_get_image_width ( png_ptr, info_ptr );
			const png_uint_32 height = png_get_image_height ( png_ptr, info_ptr );
			data::Chunk ret = data::MemChunk<TYPE >( width, height );

			/* png needs a pointer to each row */
			boost::scoped_array<png_bytep> row_pointers( new png_bytep[height] );

			for ( unsigned short r = 0; r < height; r++ )
				row_pointers[r] = ( png_bytep )&ret.voxel<TYPE>( 0, r );

			png_set_strip_alpha(png_ptr);
			png_read_image( png_ptr, row_pointers.get() );
			ret.flipAlong( data::rowDim ); //the png-"space" is mirrored to the isis space
#if __BYTE_ORDER == __LITTLE_ENDIAN // png is always big endian, so we swap if we run on little endian
			ret.asValueArrayBase().endianSwap();
#endif
			return ret;
		}
	};

	template<typename T, typename DEST> bool extractNumberFromName( const boost::filesystem::path &name, DEST &property ) {
		std::string filename=name.filename().native();
		std::string::size_type end = filename.find_last_of( "0123456789" );

		if( end != std::string::npos ) {
			std::string::size_type start = filename.find_last_not_of( "0123456789", end );
			property = boost::lexical_cast<T>( filename.substr( start + 1, end - start ) );
			return true;
		} else {
			return false;
		}
	}
	std::map<png_byte, std::map<png_byte, std::shared_ptr<Reader> > > readers;
public:
	ImageFormat_png() {
		readers[PNG_COLOR_TYPE_GRAY][8].reset( new GenericReader<uint8_t> );
		readers[PNG_COLOR_TYPE_GRAY][16].reset( new GenericReader<uint16_t> );
		readers[PNG_COLOR_TYPE_RGB][8].reset( new GenericReader<util::color24> );
		readers[PNG_COLOR_TYPE_RGB][16].reset( new GenericReader<util::color48> );
		
		//the generic reader by default strips alpha, so we can use the same reader for images with/without alpha channel
		readers[PNG_COLOR_TYPE_RGB_ALPHA]=readers[PNG_COLOR_TYPE_RGB];
		readers[PNG_COLOR_TYPE_GRAY_ALPHA]=readers[PNG_COLOR_TYPE_GRAY];
	}
	std::string getName()const override {
		return "PNG (Portable Network Graphics)";
	}
	util::istring dialects( const std::list<util::istring> & ) const override {return "middle stacked";}
	bool write_png( const std::string &filename, const data::Chunk &src, int color_type, int bit_depth ) {
		assert( src.getRelevantDims() == 2 );
		FILE *fp;
		png_structp png_ptr;
		png_infop info_ptr;

		//buff has to be swapped along the png-x-axis
		data::Chunk buff = src.copyByID(); //make a deep copy to not interfere with the source
		buff.flipAlong( data::rowDim ); //the png-"space" is mirrored to the isis space @todo check if we can use exif

		util::vector4<size_t> size = buff.getSizeAsVector();

		/* open the file */
		fp = fopen( filename.c_str(), "wb" );

		if ( fp == NULL ) {
			throwSystemError( errno, std::string( "Failed to open " ) + filename );
			return 0;
		}

		/* Create and initialize the png_struct with the desired error handler
		* functions.  If you want to use the default stderr and longjump method,
		* you can supply NULL for the last three parameters.  We also check that
		* the library version is compatible with the one used at compile time,
		* in case we are using dynamically linked libraries.  REQUIRED.
		*/
		png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL /*user_error_ptr*/, NULL /*user_error_fn*/, NULL /*user_warning_fn*/ );

		if ( png_ptr == NULL ) {
			fclose( fp );
			throwSystemError( errno, "png_create_write_struct failed" );
			return 0;
		}

		/* Allocate/initialize the image information data.  REQUIRED */
		info_ptr = png_create_info_struct( png_ptr );

		if ( info_ptr == NULL ) {
			fclose( fp );
			throwSystemError( errno, "png_create_info_struct failed" );
			return 0;
		}

		/* Set error handling.  REQUIRED if you aren't supplying your own
		* error handling functions in the png_create_write_struct() call.
		*/
		if ( setjmp( png_jmpbuf( png_ptr ) ) ) {
			/* If we get here, we had a problem writing the file */
			fclose( fp );
			png_destroy_write_struct( &png_ptr, &info_ptr );
			throwSystemError( errno, std::string( "Could not write to " ) + filename );
			return false;
		}

		// check the image sizes
		if( size[data::rowDim] > png_get_user_width_max( png_ptr ) ) {
			LOG( Runtime, error ) << "Sorry the image is to wide to be written as PNG (maximum is " << png_get_user_width_max( png_ptr ) << ")";
		}

		if( size[data::columnDim] > png_get_user_height_max( png_ptr ) ) {
			LOG( Runtime, error ) << "Sorry the image is to high to be written as PNG (maximum is " << png_get_user_height_max( png_ptr ) << ")";
		}

		/* set up the output control if you are using standard C streams */
		png_init_io( png_ptr, fp );
		png_set_IHDR( png_ptr, info_ptr, ( png_uint_32 )size[0], ( png_uint_32 )size[1], bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

		/* png needs a pointer to each row */
		png_byte **row_pointers = new png_byte*[size[1]];
		row_pointers[0] = ( png_byte * )buff.getValueArrayBase().getRawAddress().get();

		for ( unsigned short r = 1; r < size[1]; r++ )
			row_pointers[r] = row_pointers[0] + ( buff.getBytesPerVoxel() * buff.getLinearIndex( { 0, r, 0, 0 } ) );

		png_set_rows( png_ptr, info_ptr, row_pointers );

		/* This is the easy way.  Use it if you already have all the
		* image info living info in the structure.  You could "|" many
		* PNG_TRANSFORM flags into the png_transforms integer here.
		*/
		png_write_png( png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL );

		/* clean up after the write, and free any memory allocated */
		png_destroy_write_struct( &png_ptr, &info_ptr );
		delete[] row_pointers;

		/* close the file */
		fclose( fp );

		/* that's it */
		return true;
	}

	data::Chunk read_png( const boost::filesystem::path &filename ) {
		png_byte header[8]; // 8 is the maximum size that can be checked

		/* open file and test for it being a png */
		FILE *fp = fopen( filename.c_str(), "rb" );

		if ( !fp ) {
			throwSystemError( errno, std::string( "Could not open " ) + filename.native() );
		}

		if( fread( header, 1, 8, fp ) != 8 ) {
			throwSystemError( errno, std::string( "Could not open " ) + filename.native() );
		}

		;

		if ( png_sig_cmp( header, 0, 8 ) ) {
			throwGenericError( filename.native() + " is not recognized as a PNG file" );
		}

		png_structp png_ptr;
		png_infop info_ptr;

		/* initialize stuff */
		png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		assert( png_ptr );

		info_ptr = png_create_info_struct( png_ptr );
		assert( info_ptr );

		png_init_io( png_ptr, fp );
		png_set_sig_bytes( png_ptr, 8 );
		png_read_info( png_ptr, info_ptr );
		png_set_interlace_handling( png_ptr );
		png_read_update_info( png_ptr, info_ptr );
		const png_byte color_type = png_get_color_type ( png_ptr, info_ptr );
		const png_byte bit_depth = png_get_bit_depth( png_ptr, info_ptr );
		std::shared_ptr< Reader > reader = readers[color_type][bit_depth];
		LOG_IF(color_type&PNG_COLOR_MASK_ALPHA,Runtime,notice) << "Ignoring alpha channel in the image";

		if( !reader ) {
			LOG( Runtime, error ) << "Sorry, the color type " << ( int )color_type << " with " << ( int )bit_depth << " bits is not supportet.";
			throwGenericError( "Wrong color type" );
		}

		data::Chunk ret = ( *reader )( png_ptr, info_ptr );

		fclose( fp );
		return ret;
	}
	std::list<data::Chunk> load(const boost::filesystem::path &filename, std::list<util::istring> /*formatstack*/, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> /*feedback*/)  throw( std::runtime_error & ) override
	{
		data::Chunk ch = read_png( filename );

		if( dialect == "stacked" ) {
			float slice;

			if( extractNumberFromName<uint32_t>( filename, slice ) ) {
				ch.setValueAs<uint32_t>( "acquisitionNumber", slice );
				ch.setValueAs( "indexOrigin", util::fvector3( {0, 0, slice} ) );
				LOG( Runtime, info ) << "Synthesized acquisitionNumber " << ch.property( "acquisitionNumber" )
									 << " and slice position " <<  ch.property( "indexOrigin" ) <<  " from filename";
				LOG( Runtime, info ) << ch.getSizeAsString() << "-image loaded from png. Making up columnVec,rowVec and voxelSize";
			}
		} else {
			if( extractNumberFromName<uint32_t>( filename, ch.touchProperty( "acquisitionNumber" ) ) ) {
				LOG( Runtime, info ) << "Synthesized acquisitionNumber " << ch.property( "acquisitionNumber" ) << " from filename";
				LOG( Runtime, notice ) << ch.getSizeAsString() << "-image loaded from png. Making up columnVec,indexOrigin,rowVec and voxelSize";
			} else {
				LOG( Runtime, notice ) << ch.getSizeAsString() << "-image loaded from png. Making up acquisitionNumber,columnVec,indexOrigin,rowVec and voxelSize";
				ch.setValueAs<uint32_t>( "acquisitionNumber", 0 );
			}

			ch.setValueAs( "indexOrigin", util::fvector3( ) );
		}


		ch.setValueAs( "sequenceNumber", ( uint16_t )1 );
		ch.setValueAs( "rowVec",    util::fvector3{1, 0, 0} );
		ch.setValueAs( "columnVec", util::fvector3{0, 1, 0} );
		ch.setValueAs( "voxelSize", util::fvector3{1, 1, 1} );
		return std::list< data::Chunk >(1, ch);
	}

	void write( const data::Image &image, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )  throw( std::runtime_error & )override {
		const short unsigned int isis_data_type = image.getMajorTypeID();

		data::Image tImg = image;

		if( image.getRelevantDims() < 2 ) { // ... make sure its made of slices
			throwGenericError( "Cannot write png when image is made of stripes" );
		}

		png_byte color_type, bit_depth; ;
		data::scaling_pair scale;
		if(image.hasProperty("window/max") && image.hasProperty("window/min")){
			auto minmax=image.getMinMax();
			const util::PropertyValue min = image.property("window/min"),max = image.property("window/max");
			scale=data::ValueArrayBase::getConverterFromTo(data::ValueArray<double>::staticID(),isis_data_type)->getScaling(min.front(),max.front());
			
			static const util::Value<uint8_t> one( 1 );
			static const util::Value<uint8_t> zero( 0 );
			LOG_IF(!(scale.first->eq(one) && scale.second->eq(zero)), Runtime,notice) << "Rescaling values with " << scale << " to fit windowing";
		}

		switch( isis_data_type ) {
		case data::ValueArray< int8_t>::staticID(): // if its signed, fall "back" to unsigned
		case data::ValueArray<uint8_t>::staticID():
			tImg.convertToType( data::ValueArray<uint8_t>::staticID(), scale ); // make sure whole image has same type   (u8bit)
			color_type = PNG_COLOR_TYPE_GRAY;
			bit_depth = 8;
			break;
		case data::ValueArray< int16_t>::staticID(): // if its signed, fall "back" to unsigned
		case data::ValueArray<uint16_t>::staticID():
			tImg.convertToType( data::ValueArray<uint16_t>::staticID(), scale ); // make sure whole image has same type (u16bit)
			color_type = PNG_COLOR_TYPE_GRAY;
			bit_depth = 16;
			break;
		case data::ValueArray<util::color24>::staticID():
		case data::ValueArray<util::color48>::staticID():
			tImg.convertToType( isis_data_type ); // make sure whole image hase same type (color24 or color48)
			color_type = PNG_COLOR_TYPE_RGB;
			bit_depth = ( png_byte )tImg.getChunk( 0, 0 ).getBytesPerVoxel() * 8 / 3;
			break;
		default:
			color_type = bit_depth = 0;
			LOG( Runtime, error ) << "Sorry, writing images of type " << image.getMajorTypeName() << " is not supportet";
			throwGenericError( "unsupported data type" );
		}

		tImg.spliceDownTo( data::sliceDim );


		if( util::istring( dialect.c_str() ) == util::istring( "middle" ) ) { //save only the middle
			std::vector<data::Chunk > chunks;
			size_t middle= tImg.getDimSize(data::sliceDim) / 2 + 1;
			LOG( Runtime, info ) 
				<< "Writing the slice " << middle << " of " << tImg.getDimSize(data::sliceDim) 
				<< " slices as png-image of size " << tImg.getChunk(0).getSizeAsString();
				
			for(int i=0;i<tImg.getDimSize(data::timeDim);i++)
				chunks.push_back(tImg.getChunk(0,0,middle,i));
			
			writeChunks(chunks,filename,color_type,bit_depth,feedback);  // write all slices

		} else { //save all slices
			writeChunks(tImg.copyChunksToVector( false ),filename,color_type,bit_depth,feedback);  // write all slices
		}

	}
	void writeChunks(std::vector<data::Chunk > chunks,std::string filename, png_byte color_type, png_byte bit_depth, std::shared_ptr<util::ProgressFeedback> feedback){
			if(feedback)
				feedback->show(chunks.size(),std::string("Writing ")+std::to_string(chunks.size())+" slices as png files");
			size_t number = 0;
			unsigned short numLen = std::log10( chunks.size() ) + 1;
			const std::pair<std::string, std::string> fname = makeBasename( filename );
			LOG( Runtime, info )
					<< "Writing " << chunks.size() << " slices as png-images " << fname.first << "_"
					<< std::string( numLen, 'X' ) << fname.second << " of size " << chunks.front().getSizeAsString();

			for( const data::Chunk & ref :  chunks ) {
				if(feedback)
					feedback->progress();
				const std::string num = std::to_string( ++number );
				const std::string name = fname.first + "_" + std::string( numLen - num.length(), '0' ) + num + fname.second;

				if( !write_png( name, ref, color_type, bit_depth ) ) {
					throwGenericError( std::string( "Failed to write " ) + name );;
				}
			}
	}
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_png();
}
