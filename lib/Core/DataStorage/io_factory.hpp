//
// C++ Interface: io_factory
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//



#ifndef IO_FACTORY_H
#define IO_FACTORY_H

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#define BOOST_FILESYSTEM_VERSION 2 //@todo switch to 3 as soon as we drop support for boost < 1.44
#include <boost/filesystem.hpp>

#include "io_interface.h"
#include "../CoreUtils/progressfeedback.hpp"

//????
#include "chunk.hpp"
#include "image.hpp"

namespace isis
{
namespace data
{

class IOFactory
{
public:
	typedef boost::shared_ptr< image_io::FileFormat> FileFormatPtr;
	typedef std::list<FileFormatPtr> FileFormatList;
	friend class util::Singletons;
	
private:
	boost::shared_ptr<util::ProgressFeedback> m_feedback;
public:
	/**
	 * Load data from a set of files or directories with given paths and dialect.
	 * @param paths list if files or directories to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of images created from the loaded data
	 * @note the images a re created from all loaded files, so loading mutilple files can very well result in only one image
	 */
	static std::list<data::Image> load( const util::slist &paths, util::istring suffix_override = "", util::istring dialect = "" );
	/**
	 * Load a data file or directory with given filename and dialect.
	 * @param path file or directory to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of images created from the loaded data
	 */
	static std::list<data::Image> load( const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );
	/**
	 * Load a data file with given filename and dialect into a chunklist.
	 * @param chunks list to store the loaded chunks in
	 * @param path file or directory to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	static size_t load( std::list<data::Chunk> &chunks, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );

	static bool write( const data::Image &image, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );
	static bool write( std::list<data::Image> images, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );

	/// Get a list of all known file-formats (aka. io-plugins loaded)
	static FileFormatList getFormats();

	static void setProgressFeedback( boost::shared_ptr<util::ProgressFeedback> feedback );

	/**
	 * Get all formats which should be able to read/write the given file.
	 * \param filename the file which should be red/written
	 * \param suffix_override if given, it will override the suffix of the given file (and thus enforce usage of a format)
	 * \param dialect if given, the plugins supporting the dialect are preferred
	 */
	static FileFormatList getFileFormatList( std::string filename, util::istring suffix_override = "", util::istring dialect = "" );
	/**
	 *  Make images out of a (unordered) list of chunks.
	 *  Uses the chunks in the chunklist to fit them together into images.
	 *  This removes _every_ image from chunks - so make a copy if you need them
	 *  \param chunks list of chunks to be used for the new images.
	 *  \returns a list of newly created images consisting off chunks out of the given chunk list.
	 */
	static std::list<data::Image> chunkListToImageList( std::list<Chunk> &chunks );
protected:
	size_t loadFile( std::list<Chunk> &ret, const boost::filesystem::path &filename, util::istring suffix_override = "", util::istring dialect = "" );
	size_t loadPath( std::list<Chunk> &ret, const boost::filesystem::path &path, util::istring suffix_override = "", util::istring dialect = "" );

	static IOFactory &get();
	IOFactory();//shall not be created directly
	FileFormatList io_formats;

	/*
	 * each ImageFormat will be registered in a map after plugin has been loaded
	 * @param plugin pointer to the plugin to register
	 *
	 * @return true if registration was successful, false otherwise
	 * */
	bool registerFileFormat( const FileFormatPtr plugin );
	unsigned int findPlugins( const std::string &path );
private:
	/**
	 * Stores a map of suffixes to a list FileFormats which supports this suffixes.
	 * Leading "." are stripped in the suffixes.
	 */
	std::map<util::istring, FileFormatList> io_suffix;
	IOFactory &operator =( IOFactory & ); //dont do that
};

}
}

#endif //IO_FACTORY_H
