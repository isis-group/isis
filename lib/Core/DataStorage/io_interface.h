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
#include <boost/system/system_error.hpp>
#include "image.hpp"
#include "common.hpp"
#include "../CoreUtils/istring.hpp"

namespace isis
{
namespace image_io
{

/// Base class for image-io-plugins
class FileFormat
{
protected:
	/**
	 * Check if a given property exists in the given PropMap.
	 * If the property doesn't exist a message will be sent to Log using the given loglevel.
	 * \returns object.hasProperty(name)
	 */
	static bool hasOrTell( const util::PropMap::pname_type &name, const util::PropMap &object, LogLevel level );
	/**
	 * Transform a given property into another and remove the original in the given PropMap.
	 * If the property doesn't exist a message will be sent to Log using the given loglevel.
	 * \returns true if the property existed and was transformed.
	 */
	template<typename TYPE> static bool
	transformOrTell( const util::PropMap::pname_type &from, const util::PropMap::pname_type &to, util::PropMap &object, LogLevel level ) {
		if ( hasOrTell( from, object, level ) and object.transform<TYPE>( from, to ) ) {
			LOG( Debug, verbose_info ) << "Transformed " << from << " into " << object.propertyValue( to );
			return true;
		}

		return false;
	}
	/// \return the file-suffixes the plugin supports
	virtual std::string suffixes()const = 0;
public:
	static void throwGenericError( std::string desc );
	static void throwSystemError( int err, std::string desc = "" );
	boost::filesystem::path plugin_file;

	/// splits the suffix (and the ".") from the filename (or path) and returns a pair made of both parts
	virtual std::pair<std::string, std::string> makeBasename( const std::string &filename )const;

	static std::string makeFilename( const util::PropMap &img, std::string namePattern );
	std::list<std::string> makeUniqueFilenames( const data::ImageList &images, const std::string &namePattern )const;


	static const float invalid_float;
	/// \return the name of the plugin
	virtual std::string name()const = 0;
	/**
	 * get all file suffixes a plugin suggests to handle
	 * The string returned by suffixes is tokenized at the spaces and every leading "." is stripped.
	 * The result is returned in a string-list sorted by the length of the suffix (longest first).
	 * @param reader the plugin to ask
	 * @return a list of suffixes the plugin handles
	 */
	std::list<util::istring> getSuffixes()const;


	/// \return the dialects the plugin supports
	virtual std::string dialects( const std::string &filename )const {return std::string();};
	/// \return if the plugin is not part of the official distribution
	virtual bool tainted()const {return true;}
	/**
	 * Load data into the given chunk list.
	 * I case of an error std::runtime_error will be thrown.
	 * \param chunks the chunk list where the loaded chunks shall be added to
	 * \param filename the name of the file to load from (the system does NOT check if this file exists)
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 * \returns the amount of loaded chunks.
	 */
	virtual int load( data::ChunkList &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) = 0; //@todo should be locked
	/**
	 * Write a single image to a file.
	 * I case of an error std::runtime_error will be thrown.
	 * \param filename the name of the file to write (the system does NOT check if this file exists/is writeable)
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 */
	virtual void write( const data::Image &image, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) = 0;
	/**
	 * Write a image list.
	 * I case of an error std::runtime_error will be thrown.
	 * The default implementation will call write( const data::Image &, const std::string&, const std::string&) for every image using a generated unique filename.
	 * \param filename the name to be used as base for the filename generation if neccessary.
	 * \param dialect the dialect to be used when loading the file (use "" to not define a dialect)
	 */
	virtual void write( const data::ImageList &images, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & );
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
