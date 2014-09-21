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

#include "../CoreUtils/application.hpp"
#include "../CoreUtils/progressfeedback.hpp"
#include "image.hpp"

namespace isis
{
namespace data
{

class IOApplication: public util::Application
{
	bool m_input;
	template< typename TYPE > std::list<data::TypedImage<TYPE> > convertTo( const std::list<data::Image> &src ) {
		return std::list<data::TypedImage<TYPE> >( src.begin(), src.end() );
	}
	boost::shared_ptr<util::ConsoleFeedback> feedback;

public:
	std::list<data::Image> images;
	/**
	 * Add input parameters to the given ParameterMap.
	 * This adds the usual parameter for image input (used by autoload( const util::ParameterMap &, std::list< Image >&, bool, const std::string &, boost::shared_ptr< util::ConsoleFeedback > ) ) to the given ParameterMap. The parameters are:
	 * - \c -in the filename or path to load from
	 * - \c -rf to override the file suffix used to select the plugin used for reading
	 * - \c -rdialect selects a special dialect used for reading
	 * \param parameters the ParameterMap the parameters should be added to
	 * \param needed if true, the -in parameter is marked as needed (init will fail, if this is not set)
	 * \param suffix text to be appended to the parameters above (eg. "1" here results in "-in1" etc.) to distinguish multiple inputs
	 * \param desc a specific description of this particular input parameters (will be used in the help-output)
	 */
	static void addInput ( util::ParameterMap &parameters, bool needed = true, const std::string &suffix = "", const std::string &desc = "" );
	/**
	 * Add output parameters to the given ParameterMap.
	 * This adds the usual parameters for image output to the given ParameterMap.
	 * The parameters are:
	 * - \c -out the filename to write to
	 * - \c -wf to override the file suffix used to select the plugin used for writing
	 * - \c -wdialect selects a special dialect used for writing
	 * - \c -repn selects a data type used for writing
	 * - \c -scale_mode selects the scaling strategy when converting data type for writing
	 * \param parameters the ParameterMap the parameters should be added to
	 * \param needed if true, the -out parameter is marked as needed (init will fail, if this is not set)
	 * \param suffix text to be appended to the parameters above (eg. "1" here results in "-out1" etc.) to distinguish multiple outputs
	 * \param desc a specific description of this particular output parameters (will be used in the help-output)
	 */
	static void addOutput( util::ParameterMap &parameters, bool needed = true, const std::string &suffix = "", const std::string &desc = "" );

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
	 * that all chunks of this image are of the given type (using convertToType).
	 * If the input image list is empty, an exception is thrown.
	 * You might want to check the amount of available images via images.size().
	 * \param copy enforce deep copy of the data, even if its not neccessary
	 * \returns the currently first image in the input chain represented in the given type
	 */
	template<typename TYPE> TypedImage<TYPE> fetchImageAs( bool copy = true ) {
		return copy ? MemImage<TYPE>( fetchImage() ) : TypedImage<TYPE>( fetchImage() );
	}

	/** Load data into the input list of the Application using the internal ParameterMap.
	 * This is the default loading function, it is calling autoload( const util::ParameterMap &, std::list< Image >&, bool, const std::string &, boost::shared_ptr< util::ConsoleFeedback > )
	 * with the ParameterMap of the application and stores the loaded image in its input list (see fetchImage and fetchImageAs).
	 * \note usually there is no nedd to explicitely call that function. It is called automatically by init() if the Application is set up for input (see IOApplication()).
	 */
	bool autoload( bool exitOnError = false );

	/**
	 * Load images using parameters from the given ParameterMap.
	 * These program parameters should be the same as set by addInput().
	 * \param parameters the ParameterMap the parameters should be taken from
	 * \param images an image list to store the red images
	 * \param exitOnError terminate the application if there was an error reading the data
	 * \param suffix the same suffix used for addInput()
	 * \param feedback if given, the util::ConsoleFeedback object will be used to display reading progress if possible
	 */
	static bool autoload( const util::ParameterMap &parameters, std::list< Image >& images, bool exitOnError = false, const std::string &suffix = "", boost::shared_ptr< util::ConsoleFeedback > feedback = boost::shared_ptr< util::ConsoleFeedback >() );

	/** Write data using the internal ParameterMap.
	 * Uses autowrite(const util::ParameterMap &, Image, bool, const std::string &, boost::shared_ptr< util::ConsoleFeedback >) with the ParameterMap of the application to write the given image.
	 * \param out_image the image to be written
	 * \param exitOnError terminate the application if there was an error writing the data
	 */
	bool autowrite( Image out_image, bool exitOnError = false );
	/// \overload autowrite(Image, bool)
	bool autowrite( std::list<data::Image> out_images, bool exitOnError = false );

	/**
	 * Write images using parameters from the given ParameterMap.
	 * These program parameters should be the same as set by addOutput().
	 * \param parameters the ParameterMap the parameters should be taken from
	 * \param out_image the image to be written
	 * \param exitOnError terminate the application if there was an error writing the data
	 * \param suffix the same suffix used for addOutput()
	 * \param feedback if given, the util::ConsoleFeedback object will be used to display reading progress if possible
	 */
	static bool autowrite( const util::ParameterMap &parameters, Image out_image, bool exitOnError = false, const std::string &suffix = "", boost::shared_ptr< util::ConsoleFeedback > feedback = boost::shared_ptr< util::ConsoleFeedback >() );
	/// \overload autowrite( const util::ParameterMap &, Image, bool, const std::string &, boost::shared_ptr< util::ConsoleFeedback > feedback)
	static bool autowrite( const util::ParameterMap &parameters, std::list< Image > out_images, bool exitOnError = false, const std::string &suffix = "", boost::shared_ptr< util::ConsoleFeedback > feedback = boost::shared_ptr< util::ConsoleFeedback >() );

protected:
	virtual boost::shared_ptr<util::MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;
};

}
}

#endif // DATA_IOAPPLICATION_HPP
