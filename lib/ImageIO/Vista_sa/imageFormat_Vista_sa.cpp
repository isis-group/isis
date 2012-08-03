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
#include "VistaSaParser.hpp"
#include <time.h>
#include "boost/date_time/posix_time/posix_time.hpp"


namespace isis
{

namespace image_io
{

void ImageFormat_VistaSa::sanitize( util::PropertyMap &obj )
{
	if( obj.hasProperty( "vista/columnVec" ) && obj.hasProperty( "vista/rowVec" ) ) { // if we have the complete orientation
		obj.transform<util::fvector3>( "vista/columnVec", "rowVec" );
		obj.transform<util::fvector3>( "vista/rowVec", "columnVec" );
		transformOrTell<util::fvector3>( "vista/sliceVec", "sliceVec", obj, warning );
	} else if( obj.hasProperty( "vista/imageOrientationPatient" ) ) {
		const util::dlist vecs = obj.getPropertyAs<util::dlist>( "vista/imageOrientationPatient" ); // if we have the dicom style partial orientation
		util::dlist::const_iterator begin = vecs.begin(), middle = vecs.begin(), end = vecs.end();
		std::advance( middle, 3 );
		obj.setPropertyAs( "rowVec",    util::fvector3() ).castTo<util::fvector3>().copyFrom( begin, middle );
		obj.setPropertyAs( "columnVec", util::fvector3() ).castTo<util::fvector3>().copyFrom( middle, end );
	} else { // if we dont have an orientation
		obj.setPropertyAs( "rowVec",    util::fvector3( 1, 0 ) );
		obj.setPropertyAs( "columnVec", util::fvector3( 0, 1 ) );
		obj.setPropertyAs( "sliceVec",  util::fvector3( 0, 0, -1 ) );
		obj.setPropertyAs( "vista/no_geometry",  true );
		LOG( Runtime, warning ) << "No orientation info was found, assuming identity matrix";
	}

	if(
		transformOrTell<util::fvector3>( "vista/imagePositionPatient", "indexOrigin", obj, info ) ||
		transformOrTell<util::fvector3>( "vista/indexOrigin", "indexOrigin", obj, warning )
	);
	else {
		LOG( Runtime, warning ) << "No position info was found, assuming 0 0 0";
		obj.setPropertyAs( "indexOrigin", util::fvector3( 0, 0 ) );
	}

	if( transformOrTell<util::fvector3>( "vista/lattice", "voxelSize", obj, info ) ) { // if we have lattice
		if( hasOrTell( "vista/voxel", obj, info ) ) { // use that as voxel size, and the difference to voxel as voxel gap
			obj.setPropertyAs<util::fvector3>( "voxelGap", obj.getPropertyAs<util::fvector3>( "vista/voxel" ) - obj.getPropertyAs<util::fvector3>( "voxelSize" ) ) ;
			obj.remove( "vista/voxel" );
		}
	} else if( transformOrTell<util::fvector3>( "vista/voxel", "voxelSize", obj, warning ) ) {
	} else
		obj.setPropertyAs( "voxelSize", util::fvector3( 1, 1, 1 ) );

	if( obj.hasProperty( "vista/diffusionBValue" ) ) {
		const float len = obj.getPropertyAs<float>( "vista/diffusionBValue" );

		if( len > 0 && transformOrTell<util::fvector3>( "vista/diffusionGradientOrientation", "diffusionGradient", obj, warning ) ) {
			util::fvector3 &vec = obj.propertyValue( "diffusionGradient" ).castTo<util::fvector3>();
			vec.norm();
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
		gender.set( obj.getPropertyAs<std::string>( "vista/sex" ).c_str() );

		if( ( int )gender ) {
			obj.setPropertyAs( "subjectGender", gender );
			obj.remove( "vista/sex" );
		}
	}

	try{
		if(hasOrTell("vista/date",obj,warning)){
			const std::string dString=obj.getPropertyAs<std::string>("vista/date");
			int iDay,iMonth,iYear;
			sscanf(dString.c_str(),"%d.%d.%d",&iDay,&iMonth,&iYear);
			boost::gregorian::date date(iYear,iMonth,iDay);
			boost::posix_time::time_duration time;
			
			if(hasOrTell("vista/time",obj,warning)){
				time = boost::posix_time::duration_from_string(obj.getPropertyAs<std::string>("vista/time"));
				if(!time.is_not_a_date_time())
					obj.remove("vista/time");
			}
			if(!date.is_not_a_date()){
				obj.remove("vista/date");
				obj.setPropertyAs("sequenceStart",boost::posix_time::ptime(date,time));
			}
		}
	} catch(...){
		LOG(Runtime,warning) << "Failed to parse date/time pair " << obj.propertyValue("vista/date") << "/" << obj.propertyValue("vista/time");
	}

	transformOrTell<uint16_t>( "vista/seriesNumber", "sequenceNumber", obj, warning );
	transformOrTell<float>( "vista/echoTime", "echoTime", obj, warning );

	tm buff;
	util::istring date_string;
	memset( &buff, 0, sizeof( tm ) );

	if( hasOrTell( "vista/date", obj, info ) ) {
		strptime( obj.getPropertyAs<std::string>( "vista/date" ).c_str(), "%d.%m.%Y", &buff );
		date_string = "vista/date";
	}

	if( hasOrTell( "vista/time", obj, info ) ) {
		strptime( obj.getPropertyAs<std::string>( "vista/time" ).c_str(), "%T", &buff );
		date_string = "vista/time";
		obj.remove( "vista/time" );
		obj.setPropertyAs( "sequenceStart", boost::posix_time::ptime_from_tm( buff ) );
	}

	if( !date_string.empty() )try {
			obj.setPropertyAs( "sequenceStart", boost::posix_time::ptime_from_tm( buff ) );
			obj.remove( "vista/date" );
		} catch( std::out_of_range &e ) {
			LOG( Runtime, warning ) << "Failed to parse " << std::make_pair( date_string, obj.propertyValue( date_string ) ) << " " << e.what();
		}
}


int ImageFormat_VistaSa::load( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> /*progress*/  ) throw( std::runtime_error & )
{
	data::FilePtr mfile ( filename );

	if ( !mfile.good() ) {
		if ( errno ) {
			throwSystemError ( errno, filename + " could not be opened" );
			errno = 0;
		} else
			throwGenericError ( filename + " could not be opened" );
	}

	//parse the vista header
	data::ValueArray< uint8_t >::iterator data_start = mfile.begin();
	util::PropertyMap root_map;
	std::list<util::PropertyMap> ch_list;
	size_t old_size = chunks.size();

	if ( _internal::parse_vista( data_start, mfile.end(), root_map, ch_list ) ) {

		std::list<_internal::VistaInputImage> groups;
		groups.push_back( _internal::VistaInputImage( mfile, data_start ) );

		BOOST_FOREACH( const util::PropertyMap & chMap, ch_list ) {
			util::PropertyMap root;
			root.branch( "vista" ) = chMap;
			sanitize( root );

			if( !groups.back().add( root ) ) { //if current ProtoImage doesnt like
				groups.push_back( _internal::VistaInputImage( mfile, data_start ) ); // try a new one
				assert( groups.back().add( root ) ); //a new one should always work
			}
		}
		LOG( Runtime, info ) << "Parsing vista succeeded " << groups.size() << " chunk-groups created";

		uint16_t sequence = 0;
		BOOST_FOREACH( _internal::VistaInputImage & group, groups ) {
			if( group.isFunctional() )
				group.transformFromFunctional();
			else
				group.fakeAcqNum(); // we have to fake the acquisitionNumber

			group.store( chunks, root_map, sequence++ ); //put the chunk group into the output
		}
		return chunks.size() - old_size;
	} else {
		LOG( Runtime, error ) << "Parsing vista failed";
		return -1;
	}
}

void ImageFormat_VistaSa::write( const data::Image &, const std::string &, const util::istring &, boost::shared_ptr< util::ProgressFeedback > )  throw( std::runtime_error & ){}
void ImageFormat_VistaSa::write(const std::list< data::Image >& images, const std::string& filename, const util::istring& dialect, boost::shared_ptr< util::ProgressFeedback > progress)throw( std::runtime_error & )
{
	std::list<_internal::VistaOutputImage> vimages(images.begin(),images.end());
}

}

}

isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_VistaSa();
}
