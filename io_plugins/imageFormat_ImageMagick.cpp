#include <isis/data/io_interface.h>
#include <Magick++.h>
#include <stdio.h>
#include <list>

namespace isis
{
namespace image_io
{

class ImageFormat_ImageMagic: public FileFormat
{
	std::list<Magick::CoderInfo> coderList;
protected:
	util::istring suffixes( io_modes modes)const {
		util::istring ret;
		for(const Magick::CoderInfo &c:coderList){
			if(!c.isReadable() && (modes==both || modes==read_only))
				continue;
			if(!c.isWritable() && (modes==both || modes==write_only))
				continue;
			ret=ret+" "+c.name().c_str();
		}
		return ret;
	}

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
	data::Chunk image2chunk(Magick::Image &img){
		auto depth=img.depth();
		size_t rows=img.rows(),columns=img.columns();
		
		if(img.monochrome()){
			if(depth<=8){
				data::MemChunk<uint8_t> ret(columns,rows,1,1,true);
				img.write(0,0,columns,rows,"I",Magick::CharPixel,ret.asValueArrayBase().getRawAddress().get());
				return ret;
			} else if(depth<=16){
				data::MemChunk<uint16_t> ret(columns,rows,1,1,true);
				img.write(0,0,columns,rows,"I",Magick::ShortPixel,ret.asValueArrayBase().getRawAddress().get());
				return ret;
			} else {
				data::MemChunk<uint32_t> ret(columns,rows,1,1,true);
				img.write(0,0,columns,rows,"I",Magick::IntegerPixel,ret.asValueArrayBase().getRawAddress().get());
				return ret;
			}
		} else {
			if(depth<=8){
				data::MemChunk<util::color24> ret(columns,rows,1,1,true);
				img.write(0,0,columns,rows,"RGB",Magick::CharPixel,ret.asValueArrayBase().getRawAddress().get());
				return ret;
			} else {
				data::MemChunk<util::color48> ret(columns,rows,1,1,true);
				img.write(0,0,columns,rows,"RGB",Magick::ShortPixel,ret.asValueArrayBase().getRawAddress().get());
				return ret;
			}
		}
		
	}
	bool normalize(data::Image &img){
		data::scaling_pair scale;
		std::pair<util::ValueReference, util::ValueReference> minmax;
		
		const short unsigned int isis_data_type = img.getMajorTypeID();
		if(img.hasProperty("window/max") && img.hasProperty("window/min")){
			minmax.first=img.property("window/min").front();
			minmax.second=img.property("window/max").front();
		} else 
			minmax=img.getMinMax();

		switch(isis_data_type){
		case data::ValueArray<float>::staticID():
		case data::ValueArray<double>::staticID():{//floating point images need to be between 0 and 1
			double offset = -minmax.first->as<double>();
			const double range_max = minmax.second->as<double>() + offset; //allways >=0
			const double scale_max =
				range_max != 0 ? 1 / range_max : std::numeric_limits<double>::max();

			double scale = scale_max ? scale_max : std::numeric_limits<double>::max() ;//get the smaller scaling factor which is not zero so the bigger range will fit into his domain
			offset *= scale;//calc offset for dst
			
			const data::scaling_pair scaling{util::Value<double>(scale), util::Value<double>(offset)};
		
			// will make copy if scaling is not 1/0 (image was not already normalized) -- so almost always
			return img.convertToType(isis_data_type,scaling); 
		}break;
		default:
			return img.convertToType(isis_data_type); 
		}
	}

public:
	ImageFormat_ImageMagic() {
		Magick::coderInfoList(&coderList);
#ifdef WINDOWS
		char buffer[1024];
		_getcwd(buffer,1024):
		Magick::InitializeMagick(buffer);
#endif
	}
	std::string getName()const override {return "ImageMagick";}
	util::istring dialects( const std::list<util::istring> &) const override {return "middle stacked";}
	std::list<data::Chunk> load( const boost::filesystem::path &filename, std::list<util::istring> /*formatstack*/, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> /*feedback*/ )  throw( std::runtime_error & ) override
	{
		std::list<Magick::Image> imageList; 
	
		/* read all the frames from the file */
		Magick::readImages(  &imageList, filename.native() );
		std::list<data::Chunk> ret;
	
		if(imageList.size()>1){ // if multiple images where found (e.g. animated gif)
			int acqNum=0;
			Magick::coalesceImages(&imageList,imageList.begin( ),imageList.end( ));
			for(Magick::Image &img:imageList){
				ret.push_back(image2chunk(img));
				
				if( dialect == "stacked" ){					
					ret.back().setValueAs( "indexOrigin", util::fvector3{0, 0, (float)acqNum} );
				}
				ret.back().setValueAs("acquisitionNumber",acqNum++);
			}
		} else { //just one 2D-Image
			data::Chunk ch=image2chunk(imageList.front());
			if( dialect == "stacked" ) {
				float slice;
				if( extractNumberFromName<uint32_t>( filename, slice ) ) {
					ch.setValueAs<uint32_t>( "acquisitionNumber", slice );
					ch.setValueAs( "indexOrigin", util::fvector3( {0, 0, slice} ) );
					LOG( Runtime, info ) << "Synthesized acquisitionNumber " << ch.property( "acquisitionNumber" )
										<< " and slice position " <<  ch.property( "indexOrigin" ) <<  " from filename";
					LOG( Runtime, info ) << ch.getSizeAsString() << "-image loaded from " << filename << ". Making up columnVec,rowVec and voxelSize";
				}
			} else {
				if( extractNumberFromName<uint32_t>( filename, ch.touchProperty( "acquisitionNumber" ) ) ) {
					LOG( Runtime, info ) << "Synthesized acquisitionNumber " << ch.property( "acquisitionNumber" ) << " from filename";
					LOG( Runtime, notice ) << ch.getSizeAsString() << "-image loaded from " << filename << ". Making up columnVec,indexOrigin,rowVec and voxelSize";
				} else {
					LOG( Runtime, notice ) << ch.getSizeAsString() << "-image loaded from " << filename << ". Making up acquisitionNumber,columnVec,indexOrigin,rowVec and voxelSize";
					ch.setValueAs<uint32_t>( "acquisitionNumber", 0 );
				}

				ch.setValueAs( "indexOrigin", util::fvector3( ) );
			}
			ret.push_back(ch);
		}
  
		return ret;
	}

	void write( const data::Image &image, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )  throw( std::runtime_error & ) {
		
		data::Image img=image;
		normalize(img); // force all chunks to have same type (and be 0..1 if float)
		
		if( image.getRelevantDims() < 2 ) { // ... make sure its made of slices
			throwGenericError( "Cannot write with ImageMagic when image is made of stripes" );
		}
		img.spliceDownTo(data::sliceDim);
		std::list<Magick::Image> images;
		for(const data::Chunk &ch:img.copyChunksToVector(false)){
			auto data=ch.getValueArrayBase().getRawAddress();
			Magick::Image dst;

			switch(ch.getTypeID()){
				case data::ValueArray<float>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"I",Magick::FloatPixel,data.get());
					break;
				case data::ValueArray<uint32_t>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"I",Magick::IntegerPixel,data.get());
					break;
				case data::ValueArray<uint16_t>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"I",Magick::ShortPixel,data.get());
					break;
				case data::ValueArray<uint8_t>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"I",Magick::CharPixel,data.get());
					break;
				case data::ValueArray<util::color24>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"RGB",Magick::CharPixel,data.get());
					break;
				case data::ValueArray<util::color48>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"RGB",Magick::ShortPixel,data.get());
					break;
				default:
					data=data::TypedChunk<double>(ch).getValueArrayBase().getRawAddress();//if type is not here default to double and let ImageMagic decide
				case data::ValueArray<double>::staticID():
					dst=Magick::Image(ch.getDimSize(0),ch.getDimSize(1),"I",Magick::DoublePixel,data.get());
					break;
			}
			images.push_back(dst);
		}
		
		Magick::writeImages(images.begin(),images.end(),filename);
	}
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_ImageMagic();
}


