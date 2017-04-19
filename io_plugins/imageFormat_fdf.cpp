#include <isis/util/matrix.hpp>
#include <isis/data/io_interface.h>

namespace isis
{
namespace image_io
{

class ImageFormat_fdf: public FileFormat
{
private:

protected:
	util::istring suffixes( io_modes /*modes*/ )const override {return ".fdf";}
	util::PropertyMap getProps(const char *hdr_start, const char *hdr_end){
		util::PropertyMap ret;

		auto lines= util::stringToList<std::string>(std::string(hdr_start,hdr_end),std::regex("[\r\n]+"));
		if(lines.front().substr(0,2)=="#!")
			lines.pop_front();
		
		for(const auto &prop:lines){
			auto p=getProp(prop);
			if(!p.second.isEmpty())
				ret.insert(p);
		}
		return ret;
	}
	std::pair<std::string,util::PropertyValue> getProp(const std::string &prop){
		static const std::regex r_prop=std::regex( "([^\\s]+)[\\s]+\\*?([^\\s]+)[\\s]*=[\\s]*([^;]+);[\\s]*",std::regex_constants::optimize );

		std::smatch results;
		if(std::regex_match(prop,results,r_prop,std::regex_constants::match_any)){
			std::string name=results[2], val=results[3];
			util::PropertyValue prop_val;
			if(name.back()==']'){ // its a list
				prop_val.push_back(util::stringToList<std::string>(val,std::regex("[,\\s\"]+"),std::regex("^\\{[\\s\"]*"),std::regex("[\\s\"]*\\}$")));
			} else {
				if(results[1]=="char")
					val=val.substr(1,val.length()-2); // remove the '"'
				
				prop_val.push_back(val);
			}

			LOG(Debug,info) << "Parsed " << prop << " as " << std::make_pair(name,prop_val);
			return {name,prop_val};
		} 
		LOG(Runtime,warning) << "Failed parsing " << prop;
		return std::pair<std::string,util::PropertyValue>();
	}
public:
	std::string getName()const override {return "Vnmrj reconstruction format";}

	std::list<data::Chunk> load(data::ByteArray source, std::list<util::istring> /*formatstack*/, const util::istring &/*dialect*/, std::shared_ptr<util::ProgressFeedback> /*feedback*/ )throw( std::runtime_error & ) override {
		
		static const std::map<std::string,unsigned short> type_map{
			{"float",data::ValueArray<float>::staticID()},
			{"int",data::ValueArray<int>::staticID()},
			{"short",data::ValueArray<short>::staticID()}
		};
		
		const char *hdr_start=std::static_pointer_cast<const char>(source.getRawAddress()).get();
		const char *hdr_end=std::find(hdr_start,hdr_start+source.getLength(),0x0C);

		util::PropertyMap fdf_props=getProps(hdr_start,hdr_end);
		
		auto size=extractOrTell("matrix[]",fdf_props,error)->as<util::ivector4>();
		for(auto &s:size)
			if(s==0)s=1;
		
		auto s_type=extractOrTell("storage",fdf_props,error)->as<std::string>();
		auto found_type=type_map.find(s_type);
		if(found_type==type_map.end())
			throwGenericError(std::string("Unsupported type "+s_type));
		
		auto big_endian=extractOrTell("bigendian",fdf_props,warning).get_value_or(util::Value<int>(0)).as<int>();
		auto orientation_list=extractOrTell("orientation[]",fdf_props,error)->as<util::dlist>();
		
		
		hdr_end++;
		while(*hdr_end == '\n' || *hdr_end == '\r')
			hdr_end++;
		hdr_end++;
		
		auto data=source.atByID(found_type->second,hdr_end-hdr_start,util::product(size),big_endian && (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__));
		
		data::Chunk ret(data,size[0],size[1],size[2],size[3]);
		
		ret.touchBranch("fdf").transfer(fdf_props);
		
		assert(orientation_list.size()==9);
		util::Matrix3x3<float> orientation_matrix;
		std::copy(orientation_list.begin(),orientation_list.end(),std::begin(orientation_matrix));

		ret.setValueAs("rowVec",   orientation_matrix[0]);
		ret.setValueAs("columnVec",orientation_matrix[1]);
		ret.setValueAs("sliceVec", orientation_matrix[2]);

		transformOrTell<util::fvector3>("fdf/location[]","indexOrigin",ret,info) || transformOrTell<util::fvector3>("fdf/origin[]","indexOrigin",ret,warning);
		transformOrTell<float>("fdf/TE","echoTime",ret,info);
		transformOrTell<float>("fdf/TR","repetitionTime",ret,info);
		
		transformOrTell<std::string>("fdf/studyid","subjectName",ret,info);
		transformOrTell<std::string>("fdf/sequence","sequenceDescription",ret,info);
		ret.setValueAs("sequenceNumber",0);
		
		if(ret.property("fdf/rank")==2){
			transformOrTell<uint64_t>("fdf/slice_no","acquisitionNumber",ret,info);
			auto slices = ret.queryProperty("fdf/slices");
			auto array_index = ret.queryProperty("fdf/array_index");
			if(slices && array_index)
				ret.refValueAs<uint64_t>("acquisitionNumber")+= slices->as<uint64_t>() * array_index->as<uint64_t>();
		} else 
			ret.setValueAs("acquisitionNumber",0);

		if(transformOrTell<util::fvector3>("fdf/roi[]","FoV",ret,warning)){
			util::fvector3 &FoV=ret.refValueAs<util::fvector3>("FoV");
			FoV*=10;
			const util::fvector3 vsize= FoV/util::fvector3{size[0],size[1],size[2]};
			LOG(Debug, info) 
				<< "Computing voxel size from " << FoV << "(FoV)/"  << size << "(size)=" << vsize;
			ret.setValueAs("voxelSize",vsize);
		} else {
			ret.setValueAs<util::fvector3>("voxelSize",{1,1,1});	
			LOG(Runtime,warning) << "We have no roi, so we set the voxelSize to the default " << ret.property("voxelSize");
		}


		return std::list<data::Chunk>(1,ret);
	}

	void write( const data::Image &/*image*/, const std::string &/*filename*/, const util::istring &/*dialect*/, std::shared_ptr<util::ProgressFeedback> /*progress*/ )throw( std::runtime_error & ) {
		throw( std::runtime_error( "not yet implemented" ) );
	}
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_fdf();
}

