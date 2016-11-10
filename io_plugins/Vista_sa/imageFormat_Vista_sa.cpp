/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "imageFormat_Vista_sa.hpp"
#include "vistaprotoimage.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <isis/util/matrix.hpp>

#include "VistaParser.h"

namespace isis
{

namespace image_io
{
	
namespace _internal
{
	
	template<typename DURATION> bool fixDateTime(util::PropertyValue &prop, std::initializer_list<std::string> formats){
		std::tm t = {0,0,0,1,0,70,0,0,-1,0,nullptr};
		std::istringstream date_time(prop.as<std::string>());
		for(std::string format:formats){
// 			date_time.imbue(std::locale("de_DE.utf-8"));
			date_time >> std::get_time(&t, format.c_str());
			if(!date_time.fail()){
				time_t tt = mktime(&t);
				prop=std::chrono::time_point_cast<DURATION>(std::chrono::system_clock::from_time_t(tt));
				return true;
			}
		}
		return prop.transform<std::chrono::time_point<std::chrono::system_clock,DURATION>>();
	}
	template<typename DURATION> bool fixDateTime(util::PropertyMap &map, util::PropertyMap::PropPath path, std::initializer_list<std::string> formats){
		auto query=map.queryProperty(path);
		if(query && fixDateTime<DURATION>(*query,formats)){
			LOG(Debug,info) << "Parsed " << path << " as " << *query;
			return true;
		} else
			return false;
	}
}
const std::locale ImageFormat_VistaSa::vista_locale(std::cout.getloc(), new vista_date_facet());

void ImageFormat_VistaSa::sanitize( util::PropertyMap &obj )
{
	LOG(Debug,verbose_info) << "Sanitizing " << obj.branch("vista");
	
	const auto queryOrientationPatient = obj.queryProperty( "vista/imageOrientationPatient" );
	if( obj.hasProperty( "vista/columnVec" ) && obj.hasProperty( "vista/rowVec" ) ) { // if we have the complete orientation
		LOG(Runtime,info) << "Directly using vista/rowVec and vista/columnVec";
		obj.transform<util::fvector3>( "vista/columnVec", "rowVec" );
		obj.transform<util::fvector3>( "vista/rowVec", "columnVec" );
		transformOrTell<util::fvector3>( "vista/sliceVec", "sliceVec", obj, warning );
	} else if( obj.hasProperty( "vista/x-axis" ) && obj.hasProperty( "vista/y-axis" ) && obj.hasProperty( "vista/z-axis" ) ) { // if we have the complete orientation

		static const util::Matrix3x3<float> vista2isis{
			 0,-1, 0,
			 0, 0, 1,
			-1, 0, 0
		};
		
		obj.setValueAs("rowVec",    vista2isis * obj.getValueAs<util::fvector3>("vista/x-axis"));
		obj.setValueAs("columnVec", vista2isis * obj.getValueAs<util::fvector3>("vista/y-axis"));
		obj.setValueAs("sliceVec",  vista2isis * obj.getValueAs<util::fvector3>("vista/z-axis"));
		obj.remove("vista/x-axis");
		obj.remove("vista/y-axis");
		obj.remove("vista/z-axis");
		LOG(Runtime,info) 
			<< "Computed orientation from x/y/z-axis in vista as " 
			<< util::Matrix3x3<float>{
				obj.getValueAs<util::fvector3>("rowVec"),
				obj.getValueAs<util::fvector3>("columnVec"),
				obj.getValueAs<util::fvector3>("sliceVec")
			};
		
	} else if( queryOrientationPatient ) {
		LOG(Runtime,info) << "using vista/imageOrientationPatient " << queryOrientationPatient;
		const util::dlist vecs = queryOrientationPatient->as<util::dlist>(); // if we have the dicom style partial orientation
		util::dlist::const_iterator begin = vecs.begin(), middle = vecs.begin(), end = vecs.end();
		std::advance( middle, 3 );
		std::copy(begin,middle,std::begin(obj.refValueAsOr("rowVec",util::fvector3{})));
		std::copy(middle,end,std::begin(obj.refValueAsOr("columnVec",util::fvector3{})));
	} else { // if we dont have an orientation
		LOG( Runtime, warning ) << "No orientation info was found, assuming identity matrix";
		obj.setValueAs( "rowVec",    util::fvector3{ 1, 0, 0 } );
		obj.setValueAs( "columnVec", util::fvector3{ 0, 1, 0 } );
		obj.setValueAs( "sliceVec",  util::fvector3{ 0, 0, 1 } );
		obj.setValueAs( "vista/no_geometry",  true );
	}

	if(
		transformOrTell<util::fvector3>( "vista/imagePositionPatient", "indexOrigin", obj, info ) ||
		transformOrTell<util::fvector3>( "vista/indexOrigin", "indexOrigin", obj, info )
	);
	else {
		LOG( Runtime, warning ) << "No position info was found, assuming 0 0 0";
		obj.setValueAs( "indexOrigin", util::fvector3{ 0, 0, 0 } );
	}

	if( transformOrTell<util::fvector3>( "vista/lattice", "voxelSize", obj, info ) ) { // if we have lattice
		if( hasOrTell( "vista/voxel", obj, info ) ) { // use that as voxel size, and the difference to voxel as voxel gap
			obj.setValueAs( "voxelGap", obj.getValueAs<util::fvector3>( "vista/voxel" ) - obj.getValueAs<util::fvector3>( "voxelSize" ) ) ;
			obj.remove( "vista/voxel" );
		}
	} else if( transformOrTell<util::fvector3>( "vista/voxel", "voxelSize", obj, warning ) ) {
		LOG(Runtime,notice) << "No vista/lattice found, will use vista/voxel as voxelSize and won't compute voxelGap";
	} else
		obj.setValueAs( "voxelSize", util::fvector3{ 1, 1, 1 } );

	if( obj.hasProperty( "vista/diffusionBValue" ) ) {
		const float len = obj.getValueAs<float>( "vista/diffusionBValue" );

		if( len > 0 && transformOrTell<util::fvector3>( "vista/diffusionGradientOrientation", "diffusionGradient", obj, warning ) ) {
			util::fvector3 &vec = obj.refValueAs<util::fvector3>( "diffusionGradient" );
			util::normalize(vec);
			vec *= len;
			obj.remove( "vista/diffusionBValue" );
		}
	}

	transformOrTell<std::string>( "vista/seriesDescription", "sequenceDescription", obj, warning ) ||
	transformOrTell<std::string>( "vista/protocol", "sequenceDescription", obj, warning ) ||
	transformOrTell<std::string>( "vista/name", "sequenceDescription", obj, warning );

	transformOrTell<std::string>( "vista/patient", "subjectName", obj, warning );

	transformOrTell<std::string>( "vista/coilID", "transmitCoil", obj, verbose_info ) ||
	transformOrTell<std::string>( "vista/transmitCoil", "transmitCoil", obj, info );

	transformOrTell<uint16_t>( "vista/repetitionTime", "repetitionTime", obj, verbose_info ) ||
	transformOrTell<uint16_t>( "vista/repetition_time", "repetitionTime", obj, info );

	if( hasOrTell( "vista/sex", obj, warning ) ) {
		util::Selection gender( "male,female,other" );
		gender.set( obj.getValueAs<std::string>( "vista/sex" ).c_str() );

		if( ( int )gender ) {
			obj.setValueAs( "subjectGender", gender );
			obj.remove( "vista/sex" );
		}
	}
	
	// fix timestamps
	try{
		const auto dateQuery = obj.queryProperty("vista/date");
		const auto timeQuery = obj.queryProperty("vista/time");

		if(dateQuery && timeQuery){ // if we have time and date, try to parse it as combined string
			util::Value<std::string> date_time=dateQuery->as<std::string>()+" "+timeQuery->as<std::string>();
			obj.setValueAs("sequenceStart",date_time.as<util::timestamp>());
			obj.remove("vista/date");
			obj.remove("vista/time");
			LOG(Debug,info) << "Parsed sequenceStart from date/time as " << *obj.queryProperty("sequenceStart");
		} else if(dateQuery && _internal::fixDateTime<util::timestamp::duration>(*dateQuery,{"%H:%M:%S %d %b %Y"})){ //old format (is date actually a weird timestamp) -- eg: 15:09:18 25 Sep 2001
			obj.transform<util::timestamp>("vista/date","sequenceStart");
			LOG(Debug,info) << "Parsed sequenceStart from vista/date as " << *obj.queryProperty("sequenceStart");
		}else {
			LOG(Runtime,warning) << "Vista file lacks date and/or time information, can't set sequenceStart";
		}
	} catch(...){
		LOG(Runtime,warning) << "Failed to parse date/time pair " << obj.queryProperty("vista/date") << " " << obj.queryProperty("vista/time");
	}
	
	if ( obj.hasProperty( "vista/age" ) ) {
		obj.setValueAs( "subjectAge",obj.getValueAs<uint16_t>("vista/age")*365.2425 );
		obj.remove( "vista/age" );
	} else if(obj.hasProperty("vista/birth") && _internal::fixDateTime<util::date::duration>(obj,"vista/birth",{"%d%m%y"}) && obj.hasProperty("sequenceStart")){
		const util::date birthdate=obj.getValueAs<util::date>("vista/birth");
		const util::date measuredate= obj.getValueAs<util::date>("sequenceStart");
			
		obj.setValueAs<uint16_t>("subjectAge",std::chrono::duration_cast<std::chrono::days>(measuredate-birthdate).count());
		LOG(Runtime,info) << "Computed subjectAge from measurement date and birthdate as " << obj.getValueAs<uint16_t>("subjectAge")/365.2425 << " years";
	}

	transformOrTell<uint16_t>( "vista/seriesNumber", "sequenceNumber", obj, warning );
	transformOrTell<float>( "vista/echoTime", "echoTime", obj, warning );
}

void ImageFormat_VistaSa::unsanitize(util::PropertyMap& obj)
{
	
	static const char *renamePairs[][2]={
		{"sequenceDescription","protocol"},
		{"voxelSize","lattice"},
		{"subjectName","patient"},
		{"subjectGender","sex"},
		{"transmitCoil","CoilID"},
		{"caPos","ca"},
		{"cpPos","cp"},
		{"repetitionTime","repetition_time"}
	};

	//store sequenceStart
	if(obj.hasProperty("sequenceStart")){
		const util::timestamp stamp=obj.getValueAs<util::timestamp>("sequenceStart");
#pragma message "Implement me"
// 		setPropFormated("date",stamp.date(),obj);
// 		setPropFormated("time",stamp.time_of_day(),obj);
		obj.remove("sequenceStart");
	}

	//store birth
	if(obj.hasProperty("subjectBirth")){
		setPropFormated("birth",obj.getValueAs<util::date>("subjectBirth"),obj);
		obj.remove("subjectBirth");
	}

	if ( obj.hasProperty( "subjectAge" ) ) {
		// age in days
		uint16_t age = obj.getValueAs<uint16_t>( "subjectAge" );
		age = ( ( age / 365.2425 ) - floor( age / 365.2425 ) ) < 0.5 ?
			  floor( age / 365.2425 ) : ceil( age / 365.2425 );
		obj.setValueAs("age", age );
		obj.remove( "subjectAge" );
	}

	// "voxel" should be the voxelSize voxelGap
	obj.setValueAs<util::fvector3>("voxel",obj.getValueAs<util::fvector3>("voxelSize")+obj.getValueAs<util::fvector3>("voxelGap"));
	
	// rename some props
	for(const char** ref:renamePairs) 
		if(obj.hasProperty(ref[0]))
			obj.rename(ref[0],ref[1]);

}

std::list<data::Chunk> ImageFormat_VistaSa::load( const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback ) throw ( std::runtime_error & )
{
	data::FilePtr mfile ( filename );
	
	std::list<data::Chunk> chunks;

	if ( !mfile.good() ) {
		if ( errno ) {
			throwSystemError ( errno, filename + " could not be opened" );
			errno = 0;
		} else
			throwGenericError ( filename + " could not be opened" );
	}

	//parse the vista header

	std::list<util::PropertyMap> ch_list;
	util::PropertyMap root;
	std::ifstream stream(filename);
	vista_internal::VistaParser parser(stream,root,ch_list);
	
	if(parser.parse()!=0)
	{
		LOG( Runtime, error ) << "Parsing vista file " << filename << " failed";
		return chunks;
	} else {
		LOG(Debug,info) << "Parsed global properties " << root << " from " << filename;
	}
	
	data::ValueArray< uint8_t >::iterator data_start = mfile.begin() + stream.tellg();
	LOG(Debug,info) << "Vista data offset is " << stream.tellg();
	stream.close();

	std::list<_internal::VistaInputImage> groups;
	groups.push_back( _internal::VistaInputImage( mfile, data_start ) );

	for( const util::PropertyMap & chMap: ch_list ) {
		util::PropertyMap root;
		LOG(Debug, info) << "Sanitizing image " << chMap;
		root.touchBranch( "vista" ) = chMap;
		sanitize( root );

		if( !groups.back().add( root ) ) { //if current ProtoImage doesnt like
			groups.push_back( _internal::VistaInputImage( mfile, data_start ) ); // try a new one
			assert( groups.back().add( root ) ); //a new one should always work
		}
	}
	LOG( Runtime, info ) << "Parsing vista succeeded " << groups.size() << " chunk-groups created";

	uint16_t sequence = 0;
	if(feedback && ch_list.size()>10)
		feedback->show(ch_list.size(),std::string("Loading ") + boost::lexical_cast<std::string>(groups.size()) + " image(s) from " + filename );

	for( _internal::VistaInputImage & group: groups ) {
		if( group.isFunctional() )
			group.transformFromFunctional();
		else
			group.fakeAcqNum(); // we have to fake the acquisitionNumber

		group.store( chunks, root, sequence++,feedback ); //put the chunk group into the output
	}

	return chunks;
}


void ImageFormat_VistaSa::write( const data::Image &image, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )  throw( std::runtime_error & ){}
void ImageFormat_VistaSa::write( const std::list<data::Image> &images, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & )
{
	std::list<_internal::VistaOutputImage> vimages(images.begin(),images.end());
	std::ofstream out(filename.c_str(),std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
	out.exceptions( std::ios::badbit );

	out << "V-data 2 {" << std::endl;

	size_t offset=0;
	util::slist history;
	for(_internal::VistaOutputImage &ref:vimages)
		ref.extractHistory(history);
	if(!history.empty())
		util::listToOStream(history.begin(),history.end(),out,"\n\t","history: {\n\t","\n}\n");
		
	for(_internal::VistaOutputImage &ref:vimages)
		ref.storeHeaders(out,offset);
		
	out << "}" << std::endl << (char)0xC << std::endl;
	
	for(_internal::VistaOutputImage &ref:vimages)
		ref.storeVImages(out);
}

}

}

isis::image_io::FileFormat *factory()
{
// 	isis::util::DefaultMsgPrint::stopBelow(isis::warning);
	return new isis::image_io::ImageFormat_VistaSa();
}
