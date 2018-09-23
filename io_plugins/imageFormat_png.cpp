#include <isis/core/io_interface.h>
#include <png.h>
#include <zlib.h>
#include <stdio.h>
#include <fstream>
#include <boost/endian/buffers.hpp>
#include <boost/crc.hpp>
#include <future>

namespace isis
{
namespace image_io
{

class ImageFormat_png: public FileFormat
{
protected:
	util::istring suffixes( io_modes /*modes = both */ )const override {return ".png";}
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
	static void pngWrite(png_structp pngPtr, png_bytep data, png_size_t length) {
		std::ofstream* file = reinterpret_cast<std::ofstream*>(png_get_io_ptr(pngPtr));
		file->write(reinterpret_cast<char*>(data), length);
		if (file->bad()) {
			png_error(pngPtr, "Write error");
		}
	}
	static void pngFlush(png_structp pngPtr) {
		std::ofstream* file = reinterpret_cast<std::ofstream*>(png_get_io_ptr(pngPtr));
		file->flush();
	}


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
	std::list<util::istring> dialects() const override {return {"middle","stacked", "noflip", "parallel"};}
	bool write_png( const std::string &filename, const data::Chunk &src, int color_type, int bit_depth ) {
		assert( src.getRelevantDims() == 2 );
		FILE *fp;
		png_structp png_ptr;
		png_infop info_ptr;

		util::vector4<size_t> size = src.getSizeAsVector();

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
		row_pointers[0] = ( png_byte * )src.getValueArrayBase().getRawAddress().get();

		for ( unsigned short r = 1; r < size[1]; r++ )
			row_pointers[r] = row_pointers[0] + ( src.getBytesPerVoxel() * src.getLinearIndex( { 0, r, 0, 0 } ) );

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
		png_set_strip_alpha(png_ptr);

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
	std::list<data::Chunk> load(const boost::filesystem::path &filename, std::list<util::istring> /*formatstack*/, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> /*feedback*/) override
	{
		data::Chunk ch = read_png( filename );

		if( checkDialect(dialects,"stacked") ) {
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

	void write( const data::Image &image, const std::string &filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback )override {
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


		if( checkDialect(dialects, "middle" ) ) { //save only the middle
			std::vector<data::Chunk > chunks;
			size_t middle= tImg.getDimSize(data::sliceDim) / 2 + 1;
			LOG( Runtime, info ) 
				<< "Writing the slice " << middle << " of " << tImg.getDimSize(data::sliceDim) 
				<< " slices as png-image of size " << tImg.getChunk(0).getSizeAsString();
				
			for(int i=0;i<tImg.getDimSize(data::timeDim);i++)
				chunks.push_back(tImg.getChunk(0,0,middle,i));
			
			writeChunks(chunks,filename,color_type,bit_depth,dialects,feedback);  // write all slices

		} else { //save all slices
			writeChunks(tImg.copyChunksToVector( false ),filename,color_type,bit_depth,dialects,feedback);  // write all slices
		}

	}
	void writeChunks(std::vector<data::Chunk > chunks,std::string filename, png_byte color_type, png_byte bit_depth, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback){
			if(feedback)
				feedback->show(chunks.size(),std::string("Writing ")+std::to_string(chunks.size())+" slices as png files");
			size_t number = 0;
			unsigned short numLen = std::log10( chunks.size() ) + 1;
			const std::pair<std::string, std::string> fname = makeBasename( filename );
			LOG_IF(chunks.size()>1, Runtime, info )
					<< "Writing " << chunks.size() << " slices as png-images " << fname.first << "_"
					<< std::string( numLen, 'X' ) << fname.second << " of size " << chunks.front().getSizeAsString();

			for( data::Chunk buff :  chunks ) {
				if(feedback)
					feedback->progress();
				std::string name;
				if(chunks.size()>1){
					const std::string num = std::to_string( ++number );
					name = fname.first + "_" + std::string( numLen - num.length(), '0' ) + num + fname.second;
				} else 
					name=fname.first+fname.second;
				
				if(checkDialect(dialects,"parallel")){
					std::ofstream out(name.c_str());
					write_sa(out,buff,bit_depth,color_type);
				} else {
					if(!checkDialect(dialects,"noflip")){
						//buff has to be swapped along the png-x-axis
						buff = buff.copyByID(); //make a deep copy to not interfere with the source
						buff.flipAlong( data::rowDim ); //the png-"space" is mirrored to the isis space @todo check if we can use exif
					} 
					
					if( !write_png( name, buff, color_type, bit_depth ) ) {
						throwGenericError( std::string( "Failed to write " ) + name );;
					}
				}
			}
	}

	static png_byte* filterRows(std::vector<data::ValueArrayBase::Reference>::const_iterator row_it, size_t rows) {
		const size_t bytesPerRow = ((*row_it)->bytesPerElem() * (*row_it)->getLength() + 1);

		png_byte* filteredRows = reinterpret_cast<png_byte*>(malloc(rows * bytesPerRow));
		
		for(size_t row=0;row<rows;row++) {
			png_byte *dst=filteredRows+(row*bytesPerRow);
			data::ValueArrayBase::Reference current_row_ref=*(row_it++);
			//Add filter byte 0 to disable actual row filtering
			dst[0]=0;
			memcpy(dst+1,current_row_ref->getRawAddress().get(),bytesPerRow-1);
		}
		return filteredRows;
	}
	
	struct png_chunk_t
	{
		boost::endian::big_int32_buf_t length;
		const char *name; /* Textual chunk name with '\0' terminator ()terminator woth be written */
		const uint8_t *data;   /* Data, should not be modified on read! */
		void write(std::ofstream &outputFile)const{
			boost::crc_32_type crc;
			outputFile.write((const char*)&length,sizeof(length));
			outputFile.write(name,4);//write length and name (without terminator)
			if(length.value()){
				outputFile.write((const char*)data,length.value());

				crc.process_bytes(name,4);
				crc.process_bytes(data,length.value());
			} else 
				crc.process_bytes(name,4);
				
			boost::endian::big_int32_buf_t checksum(crc.checksum());
			outputFile.write((const char*)&checksum,sizeof(boost::endian::big_int32_buf_t));
		}
	};
	struct IHDR:png_chunk_t{
		struct {
			boost::endian::big_int32_buf_t width,height;
			uint8_t bit_depth,colour_type,compression_method,filter_method,interlace_method;
		}ihdr_data;
		IHDR(int32_t width, int32_t height,uint8_t bit_depth, uint8_t colour_type){
			name="IHDR";
			data=((uint8_t*)&ihdr_data);
			length=13;
			ihdr_data.width=width;ihdr_data.height=height;
			ihdr_data.bit_depth=bit_depth;
			ihdr_data.colour_type=colour_type;
			ihdr_data.compression_method=PNG_COMPRESSION_TYPE_DEFAULT;
			ihdr_data.interlace_method=PNG_INTERLACE_NONE;
			ihdr_data.filter_method=PNG_NO_FILTERS;
		}
	};
	struct IDAT:png_chunk_t{
		IDAT(const data::ValueArray<uint8_t> &dat){
			name="IDAT";
			png_chunk_t::data=&dat[0];
			png_chunk_t::length=dat.getLength();
		}
	};
	struct IEND:png_chunk_t{
		IEND(){
			name="IEND";
			png_chunk_t::data=nullptr;
			png_chunk_t::length=0;
		}
	};
	
	static std::pair<data::ByteArray,uLong> compress_row_set(std::vector<data::ValueArrayBase::Reference>::const_iterator row_it, size_t rows, bool finish){
		//add filter-byte (0) to all scanlines and re-concatenate them
		png_byte *filteredRows = filterRows(row_it,rows);
		
		//zlib compression
		const size_t bytesPerRow = ((*row_it)->bytesPerElem() * (*row_it)->getLength() + 1);
		const size_t deflateOutputSize=bytesPerRow * rows;
		char *deflateOutput;

		unsigned int have;
		z_stream zStream;
		zStream.zalloc = nullptr;
		zStream.zfree = nullptr;
		zStream.opaque = nullptr;
		if (deflateInit(&zStream, 9) != Z_OK) {
			throwGenericError("Not enough memory for compression");
		}
		uint8_t *output_buffer=(uint8_t*)malloc(deflateOutputSize);
		zStream.avail_in = bytesPerRow * rows;
		zStream.next_in = filteredRows;
		zStream.avail_out = deflateOutputSize;
		zStream.next_out = output_buffer;

		//Compress the image data with deflate
		deflate(&zStream, finish ? Z_FINISH : Z_FULL_FLUSH);
		assert(zStream.avail_in==0);
		assert(zStream.total_in==bytesPerRow * rows);
		
		auto ret=std::make_pair(data::ByteArray(output_buffer,deflateOutputSize-zStream.avail_out),zStream.adler);
		deflateEnd(&zStream);
		free(filteredRows);
		
		return ret;
	}
	
	void write_sa(std::ofstream &outputFile, const data::Chunk &src, png_byte bit_depth, png_byte color_type) {
		auto size=src.getSizeAsVector();
		
		const uint8_t signature[]{0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
		outputFile.write((std::ofstream::char_type*)signature,sizeof(signature));

		IHDR ihdr_chunk(// use smart conversion to check for bounds
			util::Value<uint64_t>(size[0]).as<int32_t>(),
			util::Value<uint64_t>(size[1]).as<int32_t>(),
			bit_depth,color_type);
		ihdr_chunk.write(outputFile);
		
		auto rows=src.getValueArrayBase().splice(size[0]);
		
		// compute amount of rows that fir into 100MB
		const size_t bytes_per_row=size[0]*src.getBytesPerVoxel();
		const size_t rowset_size=(100*1024*1024)/bytes_per_row;
		uLong adler32Combined=0;
		
		std::list<std::future<std::pair<data::ByteArray,uLong>>> generators;
		
		//set up asynchronous 
		for(size_t r=0;r<rows.size();r+=rowset_size){
			bool last_set=(r+rowset_size >= rows.size());
			size_t actual_rows = last_set ? rows.size()-r:rowset_size;
			generators.emplace_back(std::async(compress_row_set,rows.begin()+r,actual_rows,last_set));
		}
		
		// get and write their results
		for(size_t r=0;r<rows.size();r+=rowset_size){
			auto dat = generators.front().get();
			generators.pop_front();
			
			bool last_set=(r+rowset_size >= rows.size());
			size_t actual_rows = last_set ? rows.size()-r:rowset_size;
			adler32Combined = adler32_combine(adler32Combined, dat.second, bytes_per_row*actual_rows);

			if(r == 0) {
				IDAT(dat.first).write(outputFile);
			} else { // for all but the first chunks strip the zlib stream header
				if(last_set){ //overwrite adler32 checksum in last set 
					uint8_t *adler_pos = dat.first.end()-sizeof(adler32Combined);
					memcpy(adler_pos,&adler32Combined,sizeof(adler32Combined));
				}
				IDAT(dat.first.at<uint8_t>(2)).write(outputFile);
			}
			LOG(Runtime,info) 
				<< "Scalines " << r << " to " << r+actual_rows-1 
				<< " written, compression ratio was " << std::to_string(100-(dat.first.getLength()*100 / (bytes_per_row*actual_rows)))+"%"; 
		}
		IEND().write(outputFile);
	}
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_png();
}
