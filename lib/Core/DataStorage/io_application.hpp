/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef DATA_IOAPPLICATION_HPP
#define DATA_IOAPPLICATION_HPP

#include "CoreUtils/application.hpp"
#include "CoreUtils/progressfeedback.hpp"
#include "DataStorage/image.hpp"

namespace isis
{
namespace data
{

class IOApplication: public util::Application
{
	bool m_input, m_output;
	template< typename TYPE > ImageList convertTo( const ImageList &imageList ) {
		ImageList retList;
		BOOST_FOREACH( ImageList::const_reference listRef, imageList ) {
			retList.push_back( boost::shared_ptr<Image> ( new TypedImage<TYPE>( *listRef ) ) );
		}
		return retList;
	}
	util::ConsoleFeedback feedback;

public:
	data::ImageList images;
	IOApplication( const char name[], bool have_input = true, bool have_output = true );
	virtual ~IOApplication();
	virtual bool init( int argc, char **argv, bool exitOnError = true );
	virtual void printHelp( bool withHidden = false ) const;

	/**
	 * Get the next image from the input.
	 * This removes the currently first image in the input list, and returns a cheap copy of this image.
	 * If the input image list is empty, an exception is thrown.
	 * You might want to check the amount of available images via images.size().
	 * \returns the currently first image in the input chain
	 */
	Image fetchImage();
	/**
	 * Get the next image from the input.
	 * This uses fetchImage() to get the next image off of the input list and makes sure,
	 * that all chunks of this image are of the given type (using makeOfTypeId).
	 * If the input image list is empty, an exception is thrown.
	 * You might want to check the amount of available images via images.size().
	 * \param copy enforce deep copy of the data, even if its not neccessary
	 * \returns the currently first image in the input chain represented in the given type
	 */
	template<typename TYPE> Image fetchImageAs( bool copy = true ) {
		if( copy ) {
			return MemImage<TYPE>( fetchImage() );
		} else {
			Image ret = fetchImage();
			ret.makeOfTypeId( TypePtr<TYPE>::staticID );
			return ret;
		}
	}

	bool autoload( bool exitOnError = false );
	bool autowrite( const ImageList &out_images, bool exitOnError = false );
	bool autowrite( const Image &out_image, bool exitOnError = false );
};

}
}

#endif // DATA_IOAPPLICATION_HPP
