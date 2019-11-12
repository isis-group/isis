//
// C/C++ Interface: io_interface
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#ifdef __cplusplus
#include <string>
#include <memory>
#include <streambuf>
#include <deque>
#include "image.hpp"
#include "common.hpp"
#include "bytearray.hpp"
#include "istring.hpp"
#include "progressfeedback.hpp"

namespace isis
{
namespace image_io
{
	/**
	 * Check if a given property exists in the given PropMap.
	 * If the property doesn't exist a message will be sent to Log using the given loglevel.
	 * \returns the property's exact path if it exists, boost::none otherwise
	 */
	bool hasOrTell( const util::PropertyMap::key_type &name, const util::PropertyMap &object, LogLevel level );

	/**
	 * \copydoc hasOrTell( const util::PropertyMap::key_type &name, const util::PropertyMap &object, LogLevel level )
	 * \returns the property if it exists, boost::none otherwise
	 */
	boost::optional<util::PropertyValue> extractOrTell( const util::PropertyMap::key_type &name, util::PropertyMap &object, LogLevel level );

	/**
	 * Check if one of the given properties exists in the given PropMap.
	 * If no property exists, a message will be sent to Log using the given loglevel.
	 * \returns the first found property's exact path, boost::none otherwise
	 */
	util::PropertyMap::key_type hasOrTell( const std::initializer_list<util::PropertyMap::key_type> names, const util::PropertyMap &object, LogLevel level );

	/**
	 * \copydoc hasOrTell( const std::initializer_list<util::PropertyMap::key_type> names, const util::PropertyMap &object, LogLevel level )
	 * \returns the first found property, boost::none otherwise
	 */
	boost::optional<util::PropertyValue> extractOrTell( const std::initializer_list<util::PropertyMap::key_type> names, util::PropertyMap &object, LogLevel level );
	
	/**
	 * Transform a given property into another and remove the original in the given PropMap.
	 * If the property doesn't exist a message will be sent to Log using the given loglevel.
	 * \returns true if the property existed and was transformed.
	 */
	template<typename TYPE> bool
	transformOrTell( const util::PropertyMap::key_type &from, const util::PropertyMap::key_type &to, util::PropertyMap &object, LogLevel level ) {
		if ( hasOrTell( from, object, level ) and object.transform<TYPE>( from, to ) ) {
			LOG( Debug, verbose_info ) << "Transformed " << from << " into " << object.queryProperty( to );
			return true;
		}

		return false;
	}

/// Base class for image-io-plugins
class FileFormat
{
public:
	enum io_modes {read_only = 1, write_only = 2, both = 3};
protected:
	/// \return the file-suffixes the plugin supports
	virtual util::istring suffixes( io_modes modes = both )const = 0;
	static constexpr float invalid_float=-std::numeric_limits<float>::infinity();
public:
	static void throwGenericError( std::string desc );
	static void throwSystemError( int err, std::string desc = "" );
	boost::filesystem::path plugin_file;

	/// splits the suffix (and the ".") from the filename (or path) and returns a pair made of both parts
	virtual std::pair<std::string, std::string> makeBasename( const std::string &filename )const;

	static std::string makeFilename( const util::PropertyMap &img, std::string namePattern );
	std::list<std::string> makeUniqueFilenames( const std::list<data::Image> &images, const std::string &namePattern )const;


	/// \return the name of the plugin
	virtual std::string getName()const = 0;

	/**
	 * get all file suffixes a plugin suggests to handle
	 * @param mode the io mode you are asking for
	 * - read_only explicitely ask for reading - the plugin will give all suffixes it can read (maybe none)
	 * - write_only explicitely ask for writing - the plugin will give all suffixes it can write (maybe none)
	 * - both ask for suffixes which can be red \b or written (sould never be empty)
	 * @return a list of suffixes the plugin handles
	 */
	std::list<util::istring> getSuffixes( io_modes mode = both )const;


	/// \return a list of the dialects the plugin supports
	virtual std::list<util::istring> dialects()const {return {};};
	
	static bool checkDialect(const std::list<util::istring> &dialects,util::istring searched);

	/**
	 * Load data from file into the given chunk list.
	 * I case of an error std::runtime_error will be thrown.
	 * \param chunks the chunk list where the loaded chunks shall be added to
	 * \param filename the name of the file to load from (the system does NOT check if this file exists)
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 * \param feedback a shared_ptr to a ProgressFeedback-object to inform about loading progress. Not used if zero.
	 * \returns the amount of loaded chunks.
	 */
	virtual std::list<data::Chunk> 
	load( const boost::filesystem::path &filename, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback ); //@todo should be locked

	/**
	 * Load data from stream into the given chunk list.
	 * I case of an error std::runtime_error will be thrown.
	 * \param chunks the chunk list where the loaded chunks shall be added to
	 * \param filename the name of the file to load from (the system does NOT check if this file exists)
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 * \param feedback a shared_ptr to a ProgressFeedback-object to inform about loading progress. Not used if zero.
	 * \returns the amount of loaded chunks.
	 */
	virtual std::list<data::Chunk> 
	load(std::streambuf *source, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback ); //@todo should be locked

	/**
	 * Load data from memory into the given chunk list.
	 * I case of an error std::runtime_error will be thrown.
	 * \param chunks the chunk list where the loaded chunks shall be added to
	 * \param filename the name of the file to load from (the system does NOT check if this file exists)
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 * \param feedback a shared_ptr to a ProgressFeedback-object to inform about loading progress. Not used if zero.
	 * \returns the amount of loaded chunks.
	 */
	virtual std::list<data::Chunk> 
	load(data::ByteArray source, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback ); //@todo should be locked

	/**
	 * Write a single image to a file.
	 * I case of an error std::runtime_error will be thrown.
	 * \param image the image to be written
	 * \param filename the name of the file to write (the system does NOT check if this file exists/is writeable)
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 * \param feedback a shared_ptr to a ProgressFeedback-object to inform about loading progress. Not used if zero.
	 */
	virtual void write( const data::Image &image, const std::string &filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback ) = 0;

	/**
	 * Write a image list.
	 * I case of an error std::runtime_error will be thrown.
	 * The default implementation will call write( const data::Image &, const std::string&, const std::string&) for every image using a generated unique filename.
	 * \param images a list of the images to be written
	 * \param filename the name to be used as base for the filename generation if neccessary.
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 * \param feedback a shared_ptr to a ProgressFeedback-object to inform about loading progress. Not used if zero.
	 */
	virtual void write( const std::list<data::Image> &images, const std::string &filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback );

	virtual ~FileFormat() {}
};
}
}
#else
typedef struct FileFormat FileFormat;
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC__) || defined(__cplusplus)
#ifdef WIN32
	extern __declspec( dllexport ) isis::image_io::FileFormat *factory();
#else
	extern isis::image_io::FileFormat *factory();
#endif
#else
#ifdef WIN32
	extern __declspec( dllexport ) FileFormat *factory();
#else
	extern FileFormat *factory();
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif //IO_INTERFACE_H
