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
#include <memory>

#include "io_interface.h"
#include "../util/progressfeedback.hpp"

//????
#include "chunk.hpp"
#include "image.hpp"
#include <boost/variant.hpp>

using boost::optional;

namespace isis
{
namespace data
{

class IOFactory
{
public:
	typedef std::shared_ptr< image_io::FileFormat> FileFormatPtr;
	typedef std::list<FileFormatPtr> FileFormatList;
	typedef boost::variant<std::string, std::shared_ptr<const void>, std::basic_streambuf<char> *> LoadSource;
	friend class util::Singletons;

private:
	std::shared_ptr<util::ProgressFeedback> m_feedback;
	// use ImageIO's logging here instead of the normal data::Runtime/Debug
	typedef ImageIoLog Runtime;
	typedef ImageIoDebug Debug;
public:
	/**
	 * Load data from a set of files or directories with given paths and dialect.
	 * @param paths list if files or directories to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of images created from the loaded data
	 * @note the images a re created from all loaded files, so loading mutilple files can very well result in only one image
	 */
	static std::list<data::Image> load( const util::slist &paths, std::list<util::istring> formatstack = {}, util::istring dialect = "", optional< util::slist& > rejected=optional< util::slist& >() );
	/**
	 * Load a data file or directory with given filename and dialect.
	 * @param path file or directory to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of images created from the loaded data
	 */
	static std::list<data::Image> load( const std::string& path, std::list<util::istring> formatstack = {}, util::istring dialect = "", optional< util::slist& > rejected=optional< util::slist& >() );
	/**
	 * Load data from a file with given filename and dialect into a chunklist.
	 * @param path file or directory to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	static std::list<data::Chunk> loadChunks(
		const std::string &path,
		std::list<util::istring> formatstack = {},
		util::istring dialect = ""
	);
	/**
	 * Load data from stream with given dialect into a chunklist.
	 * @param source stream to load from
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	static std::list<data::Chunk> loadChunks(
		std::basic_streambuf<char> *source,
		std::list<util::istring> formatstack,
		util::istring dialect = ""
	);

	/**
	 * Load data from stream with given dialect into a chunklist.
	 * @param source memory pointer to load from
	 * @param length size of the data in bytes
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	static std::list<data::Chunk> loadChunks(
		std::shared_ptr<const void> source, size_t length,
		std::list<util::istring> formatstack,
		util::istring dialect = ""
	);

	static bool write( const data::Image &image, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );
	static bool write( std::list<data::Image> images, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );

	/// Get a list of all known file-formats (aka. io-plugins loaded)
	static FileFormatList getFormats();

	static void setProgressFeedback( std::shared_ptr<util::ProgressFeedback> feedback );

	/**
	 * Get all formats which should be able to read/write the given file.
	 * \param filename the file which should be red/written
	 * \param suffix_override if given, it will override the suffix of the given file (and thus enforce usage of a format)
	 * \param dialect if given, the plugins supporting the dialect are preferred
	 */
	static FileFormatList getFileFormatList(std::list<util::istring> format, util::istring dialect);
	
	static std::list<util::istring> getFormatStack( std::string filename );
	/**
	 *  Make images out of a (unordered) list of chunks.
	 *  Uses the chunks in the chunklist to fit them together into images.
	 *  This removes _every_ image from chunks - so make a copy if you need them
	 *  \param chunks list of chunks to be used for the new images.
	 *  \returns a list of newly created images consisting off chunks out of the given chunk list.
	 */
	static std::list<data::Image> chunkListToImageList( std::list<Chunk> &chunks, optional< isis::util::slist& > rejected=optional< isis::util::slist& >() );
protected:
	std::list<Chunk> loadFile( const boost::filesystem::path &filename, std::list<util::istring> formatstack = {}, util::istring dialect = "" )throw( std::runtime_error & );
	std::list<Chunk> loadStream( std::basic_streambuf<char> *source, std::list<util::istring> formatstack, util::istring dialect = "" )throw( std::runtime_error & );
	std::list<Chunk> loadMem( std::shared_ptr<const void> source, size_t length, std::list<util::istring> formatstack, util::istring dialect = "" )throw( std::runtime_error & );
	std::list<Chunk> loadPath(const boost::filesystem::path& path, std::list<util::istring> formatstack = {}, util::istring dialect="", optional< util::slist& > rejected=optional< util::slist& >());

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
