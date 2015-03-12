/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  Enrico Reimer <reimer@cbs.mpg.de>

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

#define BOOST_SPIRIT_DEBUG_PRINT_SOME 100
#define BOOST_SPIRIT_DEBUG_INDENT 5

#include <boost/foreach.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <isis/CoreUtils/vector.hpp>
#include "imageFormat_nifti_dcmstack.hpp"
#include "imageFormat_nifti_sa.hpp"
#include <CoreUtils/value_base.hpp>

namespace isis
{
namespace image_io
{

using boost::posix_time::ptime;
using boost::gregorian::date;
	
namespace _internal
{

extern const char dcmmeta_root[] = "DcmMeta";
extern const char dcmmeta_global[] = "DcmMeta/global";
extern const char dcmmeta_perslice_data[] = "DcmMeta/global/slices";
extern const char dcmmeta_const_data[] = "DcmMeta/global/const";

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

typedef BOOST_TYPEOF( ascii::space | '\t' | boost::spirit::eol ) SKIP_TYPE;
typedef data::ValueArray< uint8_t >::iterator ch_iterator;
typedef qi::rule<ch_iterator, JsonMap(), SKIP_TYPE>::context_type JsonMapContext;

JsonMap::JsonMap( const util::PropertyMap& src ):util::PropertyMap(src){}
void JsonMap::WriteJson( std::ostream& out )
{
}

void JsonMap::WriteSubtree( const std::map<util::istring, util::_internal::treeNode>& src, std::ostream &out )
{
	for ( const_iterator i = src.begin(); i != src.end(); i++ ) {
		
		if ( i->second.is_leaf()  ) {
			out << i->first << ":"; // write the name
			util::ValueBase& value=*(i->second.getLeaf()[0]);
			// json only differs between numbers, strings and lists of that
			if(value.fitsInto( util::Value<double>::staticID ))// numbers are written as they are
				out << value;
			else if(value.fitsInto( util::Value<util::ilist>::staticID )){ // integer lists [ x,y,z ]
				util::ilist list=value.as<util::ilist>(); //there could an integer that does not fit into double so we can't use integer lists as fp lists
				util::listToOStream(list.begin(),list.end(),out,",","[","]");
			} else if(value.fitsInto( util::Value<util::dlist>::staticID )){ // floating point lists [ x.1,y.2,z.3 ]
				util::dlist list=value.as<util::dlist>();
				util::listToOStream(list.begin(),list.end(),out,",","[","]");
			} else if(value.fitsInto( util::Value<util::slist>::staticID )){ // string lists [ "x","y","z" ]
				util::slist list=value.as<util::slist>();
				util::listToOStream(list.begin(),list.end(),out,"\",\"","[\"","\"]");
			} else {
				out << "\"" << value << "\""; // write everything else as string
			}
			out << std::endl; 
		} else {
			out << i->first << "{";
			WriteSubtree(i->second.getBranch(),out);
			out << "}" << std::endl;
		}
	}
	
	
}



struct add_member {
	char extra_token;
	add_member( char _extra_token ): extra_token( _extra_token ) {}
	void operator()( const fusion::vector2<std::string, JsonMap::value_cont> &a, JsonMapContext &context, bool & )const {
		const JsonMap::PropPath label = extra_token ?
				util::stringToList<JsonMap::KeyType>( a.m0, extra_token ) :
				JsonMap::PropPath( a.m0.c_str() );
		JsonMap &target = context.attributes.car;

		if( target.hasBranch( label ) || target.hasProperty( label ) )
			LOG( Runtime, error ) << "There is already an entry " << target << " skipping this one" ;
		else
			target.insertObject(label,a.m1);
	}
};

struct vec_trans { // insert subarrays as vector-Properties into an array of Properties
template<typename T, typename CONTEXT> void operator()( const std::vector<T> &a, CONTEXT &context, bool & )const {
	std::list<util::PropertyValue> &cont = context.attributes.car;
	util::vector4<T> buff;buff.copyFrom(a.begin(),a.end());
	cont.push_back(buff);
}
};


template<typename T> struct read {typedef qi::rule<ch_iterator, T(), SKIP_TYPE> rule;};

void JsonMap::insertObject(const JsonMap::PropPath &label, const JsonMap::value_cont& container )
{
	switch( container.which() ) {
		case 2:{
			const std::list<util::PropertyValue> &src=boost::get<std::list<util::PropertyValue> >( container );
			propertyValueVec( label )=std::vector<util::PropertyValue>(src.begin(),src.end());
			}break;
		case 1:
			branch( label ) = boost::get<JsonMap>( container );
			break;
		case 0:
			propertyValue( label ) = boost::get<util::PropertyValue>( container );
			break;
	}
}


bool JsonMap::ReadJson( data::ValueArray< uint8_t > stream, char extra_token )
{
	using qi::lit;
	using namespace boost::spirit;

	int version;
	read<value_cont>::rule value;
	read<std::string>::rule string( lexeme['"' >> *( ascii::print - '"' ) >> '"'], "string" ), label( string >> ':', "label" );
	read<fusion::vector2<std::string, value_cont> >::rule member( label >> value, "member" );
	read<JsonMap>::rule object( lit( '{' ) >> member[add_member( extra_token )] % ',' >> '}', "object" );
	read<int>::rule integer(int_ >> !lit('.'),"integer") ;
	read<util::dlist>::rule dlist( lit( '[' ) >> double_ % ',' >> ']', "dlist" );
	read<util::ilist>::rule ilist( lit( '[' ) >> integer % ',' >> ']', "ilist" );
	read<util::slist>::rule slist( lit( '[' ) >> string  % ',' >> ']', "slist" );

	read<std::vector<util::dvector4::value_type> >::rule dvec(lit( '[' ) >>  double_ >> repeat(0,3)[ ',' >> double_] >> ']', "dvec");
	read<std::vector<util::ivector4::value_type> >::rule ivec(lit( '[' ) >>  integer >> repeat(0,3)[ ',' >> integer] >> ']', "ivec");
	
	read<std::list<util::PropertyValue> >::rule dveclist( lit( '[' ) >> dvec[vec_trans()] % ',' >> ']', "dveclist");
	read<std::list<util::PropertyValue> >::rule iveclist( lit( '[' ) >> ivec[vec_trans()] % ',' >> ']', "iveclist");

	value = string | integer | double_ | slist | ilist | dlist | dveclist | iveclist | object;

	qi::debug(dvec);
	qi::debug(ivec);
	qi::debug(dveclist);
	qi::debug(iveclist);
	
	data::ValueArray< uint8_t >::iterator begin = stream.begin(), end = stream.end();
	bool erg = phrase_parse( begin, end, object[phoenix::ref( *this )=_1], ascii::space | '\t' | eol );
	return end == stream.end();
}

template<typename T> bool propDemux(std::list<T> props,std::list<data::Chunk> &chunks,JsonMap::PropPath name){
	if( (props.size()%chunks.size())!=0){
		LOG(Runtime,error) << "The length of the per-slice property DcmMeta entry " << util::MSubject(name) << " does not fit the number of slices, skipping (" << props.size() << "!=" << chunks.size() << ").";
		return false;
	}
	typename std::list<T>::const_iterator prop=props.begin();
	size_t stride=props.size()/chunks.size();
	BOOST_FOREACH(data::Chunk &ch,chunks){
		if(stride==1){
			ch.setPropertyAs<T>(name,*(prop++));
		} else if(stride>1) {
			typename std::list<T>::const_iterator buff=prop;
			std::advance(prop,stride);
			std::list<T> &ref=((util::PropertyValue &)ch.setPropertyAs(name, std::list<T>())).castTo<std::list<T> >();// work around gcc bug
			ref.insert(ref.end(), buff,prop);
		}
	}
	return true;
}


void demuxDcmMetaSlices(std::list<data::Chunk> &chunks, JsonMap &dcmmeta ){
	static const JsonMap::PropPath slices(dcmmeta_perslice_data);
	if(dcmmeta.hasBranch(slices)){
		size_t stride=1;
		if(chunks.front().getRelevantDims()==4){ //if we have an 4D-Chunk
			assert(chunks.size()==1); //there can only be one
			chunks=chunks.front().autoSplice(chunks.front().getDimSize(data::sliceDim)); // replace that one by its 3D parts
		}
		for(std::list<data::Chunk>::iterator c=chunks.begin();c!=chunks.end();){ // if we have 3D chunks splice each
			if(c->getRelevantDims()>2){
				std::list< data::Chunk > insert = c->autoSplice(1);
				chunks.insert(c,insert.begin(),insert.end()); // insert spliced chunks back into list
				chunks.erase(c++); // and remove original
			} else
				++c;
		}
		BOOST_FOREACH(const JsonMap::FlatMap::value_type &ppair,dcmmeta.branch(slices).getFlatMap()){
			bool success=false;
			JsonMap::PropPath path(util::istring(_internal::dcmmeta_perslice_data)+"/"+ppair.first); // move the prop one branch up (without "slices")
			switch(ppair.second.getTypeID()){
				case util::Value<util::ilist>::staticID:success=propDemux(ppair.second.castTo<util::ilist>(),chunks,path);break;
				case util::Value<util::dlist>::staticID:success=propDemux(ppair.second.castTo<util::dlist>(),chunks,path);break;
				case util::Value<util::slist>::staticID:success=propDemux(ppair.second.castTo<util::slist>(),chunks,path);break;
				default:
					LOG(Runtime,error) << "The the type of " << path << " (" << ppair.second.getTypeName() <<  ") is not a list, skipping.";
			}
			if(success)
				dcmmeta.remove(path);
			else
				dcmmeta.rename(path,util::istring( dcmmeta_global )+"/rejected_slices/"+ppair.first);
		}
	} else {
		LOG(Debug,warning) << "demuxDcmMetaSlices called, but there is no " << slices << " branch";
	}
}

boost::posix_time::ptime parseTM( const JsonMap &map, const JsonMap::PropPath &name )
{
	short shift = 0;
	bool ok = true;
	boost::posix_time::time_duration ret;
	std::string time_str=map.getPropertyAs<std::string>(name);
	
	// Insert the ":" -- make it hh:mm:ss.frac
	if ( time_str.at( 2 ) != ':' ) {
		time_str.insert( 2, 1, ':' );
		shift++;
	}
	
	if ( ( time_str.size() > size_t( 4 + shift ) ) && ( time_str.at( 4 + shift ) != ':' ) ) {
		time_str.insert( 4 + shift, 1, ':' );
		shift++;
	}
	
	//Try standard-parser for hh:mm:ss.frac
	try {
		ret = boost::posix_time::duration_from_string( time_str.c_str() );
		ok = not ret.is_not_a_date_time();
	} catch ( std::logic_error e ) {
		ok = false;
	}
	
	if ( ok ) {
		LOG( Debug, verbose_info )<< "Parsed time for " << name << "(" <<  time_str << ")" << " as " << ret;
		return boost::posix_time::ptime( boost::gregorian::date( 1400, 1, 1 ), ret );
		//because TM is defined as time of day we dont have a day here, so we fake one
	} else
		LOG( Runtime, warning ) << "Cannot parse Time string \"" << time_str << "\" in the field \"" << name << "\"";
	
	return boost::posix_time::ptime(boost::posix_time::not_a_date_time);
}


}

void ImageFormat_NiftiSa::translateFromDcmMetaConst( util::PropertyMap& object )
{
	const util::istring const_prefix = util::istring(_internal::dcmmeta_const_data)+"/";
	_internal::JsonMap const_tree=object.branch(const_prefix);
	
	// compute sequenceStart and acquisitionTime (have a look at table C.10.8 in the standard)
	if ( hasOrTell( const_prefix + "SeriesTime", object, warning ) ) {
		ptime sequenceStart = _internal::parseTM(const_tree, "SeriesTime" );
		const_tree.remove( "SeriesTime" );
		
		const char *dates[] = {"SeriesDate", "AcquisitionDate"};
		BOOST_FOREACH( const char * d, dates ) {
			if( hasOrTell(const_prefix + d, const_tree, warning ) ) {
				sequenceStart = ptime( const_tree.getPropertyAs<date>( d ), sequenceStart.time_of_day() );
				const_tree.remove( d );
				break;
			}
		}
		
		// compute acquisitionTime from global AcquisitionTime (if its there)
		if ( hasOrTell( const_prefix + "AcquisitionTime", object, info ) ) {
			ptime acTime = _internal::parseTM( const_tree, "AcquisitionTime" );
			const_tree.remove( "AcquisitionTime" );
			
			const char *dates[] = {"AcquisitionDate", "SeriesDate"};
			BOOST_FOREACH( const char * d, dates ) {
				if( hasOrTell(const_prefix + d, const_tree, warning ) ) {
					acTime = ptime( const_tree.getPropertyAs<date>( d ), acTime.time_of_day() );
					const_tree.remove( d );
					break;
				}
			}
			
			const boost::posix_time::time_duration acDist = acTime - sequenceStart;
			const float fAcDist = float( acDist.ticks() ) / acDist.ticks_per_second() * 1000;
			LOG( Debug, verbose_info ) << "Computed acquisitionTime as " << fAcDist;
			object.setPropertyAs( "acquisitionTime", fAcDist );
		}
		
		LOG( Debug, verbose_info ) << "Computed sequenceStart as " << sequenceStart;
		object.setPropertyAs( "sequenceStart", sequenceStart );
	}
	
	transformOrTell<uint16_t>   ( const_prefix + "SeriesNumber",     "sequenceNumber",     object, warning );
	transformOrTell<uint16_t>   ( const_prefix + "PatientsAge",      "subjectAge",         object, info );
	transformOrTell<std::string>( const_prefix + "SeriesDescription","sequenceDescription",object, warning );
	transformOrTell<std::string>( const_prefix + "PatientsName",     "subjectName",        object, info );
	transformOrTell<date>       ( const_prefix + "PatientsBirthDate","subjectBirth",       object, info );
	transformOrTell<uint16_t>   ( const_prefix + "PatientsWeight",   "subjectWeigth",      object, info );
	
	transformOrTell<std::string>( const_prefix + "PerformingPhysiciansName","performingPhysician", object, info );
	transformOrTell<uint16_t>   ( const_prefix + "NumberOfAverages",        "numberOfAverages",    object, warning );
	
	if ( hasOrTell( const_prefix + "PatientsSex", object, info ) ) {
		util::Selection isisGender( "male,female,other" );
		bool set = false;
		
		switch ( const_tree.getPropertyAs<std::string>( "PatientsSex" )[0] ) {
			case 'M':
				isisGender.set( "male" );
				set = true;
				break;
			case 'F':
				isisGender.set( "female" );
				set = true;
				break;
			case 'O':
				isisGender.set( "other" );
				set = true;
				break;
			default:
				LOG( Runtime, warning ) << "Dicom gender code " << util::MSubject( object.propertyValue( const_prefix + "PatientsSex" ) ) <<  " not known";
		}
		
		if( set ) {
			object.propertyValue( "subjectGender" ) = isisGender;
			const_tree.remove( "PatientsSex" );
		}
	}
	
	transformOrTell<uint32_t>( const_prefix + "CSAImageHeaderInfo/UsedChannelMask", "coilChannelMask", object, info );
	
	// @todo figure out how DWI data are stored
	
	
	// rename DcmMeta/global/const to DICOM ... thats essentially what it is ... and others will look there for that data
	object.rename(_internal::dcmmeta_const_data,"DICOM");
}

void ImageFormat_NiftiSa::translateFromDcmMetaSlices( util::PropertyMap& object )
{
	const util::istring slices_prefix = util::istring(_internal::dcmmeta_perslice_data)+"/";
	_internal::JsonMap slices_tree=object.branch(slices_prefix);
	
	// compute acquisitionTime
	if (hasOrTell( slices_prefix + "ContentTime", object, info ) ) {
		if(object.hasProperty("sequenceStart")){
			ptime sequenceStart =object.getPropertyAs<ptime>( "sequenceStart");
			
			ptime acTime = _internal::parseTM( slices_tree, "ContentTime" );
			slices_tree.remove( "ContentTime" );
			
			if( hasOrTell(slices_prefix + "ContentDate", slices_tree, warning ) ) {
				acTime = ptime( slices_tree.getPropertyAs<date>( "ContentDate" ), acTime.time_of_day() );
				slices_tree.remove( "ContentDate" );
			}
			
			const boost::posix_time::time_duration acDist = acTime - sequenceStart;
			const float fAcDist = float( acDist.ticks() ) / acDist.ticks_per_second() * 1000;
			LOG( Debug, verbose_info ) << "Computed acquisitionTime as " << fAcDist;
			object.setPropertyAs( "acquisitionTime", fAcDist );
		} else {
			LOG(Runtime,warning) << "Don't have sequenceStart, can't compute acquisitionTime from ContentTime";
		}
	}
	
	if ( hasOrTell( slices_prefix + "ImagePositionPatient", object, info ) ) {
		const util::fvector3 from_dcmmeta= slices_tree.getPropertyAs<util::fvector3>( "ImagePositionPatient" );
		const util::fvector3 from_nifti = object.getPropertyAs<util::fvector3>( "indexOrigin" );
		if(! from_nifti.fuzzyEqual(from_dcmmeta,100)){
			LOG(Runtime,warning) << "The slice position given in " << slices_prefix + "ImagePositionPatient" << " did not fit the one computed from the nifti header ("
			<< from_dcmmeta << "!=" << from_nifti << ")";
		}
		slices_tree.remove( "ImagePositionPatient" );
		object.setPropertyAs( "indexOrigin", from_dcmmeta );
	}
	
	
	transformOrTell<uint32_t>   ( slices_prefix + "InstanceNumber", "acquisitionNumber", object, error );
	if( slices_tree.hasProperty( "AcquisitionNumber" ) && object.propertyValue( "acquisitionNumber" ) == slices_tree.propertyValue( "AcquisitionNumber" ) )
		slices_tree.remove( "AcquisitionNumber" );
	
	// rename DcmMeta/global/slices to DICOM ... thats essentially what it is ... and others will look there for that data
		util::PropertyMap::KeyList rejects=object.branch("DICOM").join(object.branch(_internal::dcmmeta_perslice_data));
		BOOST_FOREACH(util::PropertyMap::PropPath key,rejects){
			object.rename(key,util::istring( _internal::dcmmeta_global )+"/rejected_slices/"+key.back()); // and move the rejects to rejected_slices
		}
		object.remove(_internal::dcmmeta_perslice_data);
}

void ImageFormat_NiftiSa::translateToDcmMetaConst( util::PropertyMap& orig, std::ofstream &output ){
}

void ImageFormat_NiftiSa::translateToDcmMetaSlices( util::PropertyMap& orig, std::ofstream &output ){
	
}

}
}
