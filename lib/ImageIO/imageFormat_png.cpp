#include <DataStorage/io_interface.h>
#include <png.h>
#include <stdio.h>

namespace isis
{
namespace image_io
{

class ImageFormat_png: public FileFormat
{
protected:
	std::string suffixes()const {
		return std::string( ".png" );
	}
public:
	std::string name()const {
		return "PNG (Portable Network Graphics)";
	}
    std::string dialects(const std::string& filename) const{
		return "middle";
	}
	bool write_png(const std::string &filename, data::Chunk &buff){
		FILE *fp;
		png_structp png_ptr;
		png_infop info_ptr;
		assert(buff.relevantDims()==2);
		util::FixedVector<size_t,4> size = buff.sizeToVector();
		
		/* open the file */
		fp = fopen(filename.c_str(), "wb");
		if (fp == NULL){
			throwSystemError(errno,std::string("Failed to open ")+filename);
			return 0;
		}
		/* Create and initialize the png_struct with the desired error handler
		* functions.  If you want to use the default stderr and longjump method,
		* you can supply NULL for the last three parameters.  We also check that
		* the library version is compatible with the one used at compile time,
		* in case we are using dynamically linked libraries.  REQUIRED.
		*/
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL /*user_error_ptr*/, NULL /*user_error_fn*/, NULL /*user_warning_fn*/);

		if (png_ptr == NULL){
			fclose(fp);
			throwSystemError(errno,"png_create_write_struct failed");
			return 0;
		}

		/* Allocate/initialize the image information data.  REQUIRED */
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL){
			fclose(fp);
			throwSystemError(errno,"png_create_info_struct failed");
			return 0;
		}

		/* Set error handling.  REQUIRED if you aren't supplying your own
		* error handling functions in the png_create_write_struct() call.
		*/
		if (setjmp(png_jmpbuf(png_ptr))){
			/* If we get here, we had a problem writing the file */
			fclose(fp);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			throwSystemError(errno,std::string("Could not write to ") + filename);
			return false;
		}

		/* set up the output control if you are using standard C streams */
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, size[0], size[1],8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		/* png needs a pointer to each row */
		png_byte** row_pointers= new png_byte*[size[1]];
		for (unsigned short r=0; r<size[1]; r++)
			row_pointers[r]=(png_byte*)&buff.voxel<png_byte>(0,r);
		png_set_rows(png_ptr, info_ptr, row_pointers);

		/* This is the easy way.  Use it if you already have all the
		* image info living info in the structure.  You could "|" many
		* PNG_TRANSFORM flags into the png_transforms integer here.
		*/
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

		/* clean up after the write, and free any memory allocated */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete[] row_pointers;

		/* close the file */
		fclose(fp);

		/* that's it */
		return true;
	}


	int load ( data::ChunkList &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		throwGenericError("png loading is not supportted (yet)");
		return 0;
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		if(image.relevantDims()<2){
			throwGenericError("Cannot write png when image is made of stripes");
		}
		data::Image tImg(image);
		tImg.makeOfTypeId(data::TypePtr<png_byte>::staticID);
		tImg.spliceDownTo(data::sliceDim);
		std::vector<boost::shared_ptr<data::Chunk> > chunks= tImg.getChunkList();
		unsigned short numLen=std::log10(chunks.size())+1;
		size_t number=0;
		if(util::istring( dialect.c_str() )==util::istring("middle")){ //save only the middle
			LOG(Runtime,info) << "Writing the slice " << chunks.size()/2+1 << " of " << chunks.size() << " slices as png-image of size " << chunks.front()->sizeToString();
			if(!write_png(filename,*chunks[chunks.size()/2])){
				throwGenericError(std::string("Failed to write ")+filename);
			}
		} else { //save all slices
			const std::pair<std::string,std::string> fname=makeBasename(filename);
			LOG(Runtime,info)
			<< "Writing " << chunks.size() << " slices as png-images " << fname.first << "_"
			<< std::string( numLen, 'X' ) << fname.second << " of size " << chunks.front()->sizeToString();
			
			BOOST_FOREACH(const boost::shared_ptr<data::Chunk> &ref,chunks){
				const std::string num = boost::lexical_cast<std::string>( ++number );
				const std::string name=fname.first+"_"+std::string( numLen-num.length(), '0' ) + num + fname.second;

				if(!write_png(name,*ref)){
					throwGenericError(std::string("Failed to write ")+name);;
				}
			}
		}
		
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_png();
}
