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
	typedef boost::variant<boost::filesystem::path,std::basic_streambuf<char>*,ByteArray> load_source;
	friend class util::Singletons;
	class  io_error : public std::runtime_error{
		FileFormatPtr p_format;
	public:
		io_error(const char *what,FileFormatPtr format);
		IOFactory::FileFormatPtr which()const;
	};

private:
	std::shared_ptr<util::ProgressFeedback> m_feedback;
	// use ImageIO's logging here instead of the normal data::Runtime/Debug
	typedef ImageIoLog Runtime;
	typedef ImageIoDebug Debug;
	std::list<Chunk> load_impl(const load_source &v, std::list<util::istring> formatstack, util::istring dialect)throw( io_error & );
public:
	/**
	 * Load data from a set of files or directories with given paths and dialect.
	 * @param paths list if files or directories to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of images created from the loaded data
	 * @note the images a re created from all loaded files, so loading mutilple files can very well result in only one image
	 */
	static std::list<data::Image> 
	load( const util::slist &paths, std::list<util::istring> formatstack = {}, util::istring dialect = "", optional< util::slist& > rejected=optional< util::slist& >() );
	/**
	 * Load a data file or directory with given filename and dialect.
	 * @param path file or directory to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of images created from the loaded data
	 */
	static std::list<data::Image> 
	load( const load_source &source, std::list<util::istring> formatstack = {}, util::istring dialect = "", optional< util::slist& > rejected=optional< util::slist& >() );

	/**
	 * Load data from a given filename/stream/memory and dialect into a chunklist.
	 * @param path file or directory to load
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	static std::list<data::Chunk> loadChunks(const load_source &source,std::list<util::istring> formatstack = {},util::istring dialect = "")throw( io_error & );

	static bool write( const data::Image &image, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );
	static bool write( std::list<data::Image> images, const std::string &path, util::istring suffix_override = "", util::istring dialect = "" );

	/// Get a list of all known file-formats (aka. io-plugins loaded)
	static FileFormatList getFormats();

	static void setProgressFeedback( std::shared_ptr<util::ProgressFeedback> feedback );

	/**
	 * Get all formats which should be able to read/write the given format stack optionally using the given dialect.
	 * \param format_stack the maximum format stack to look for.
	 * \param dialect if given, the plugins supporting the dialect are preferred
	 * 
	 * \example getFileFormatList({"dcm","tar","gz"}, "") will look for plugins that cat read "*.dcm.tar.gz" files. 
	 * As this will likely fail it will look for plugins that can handle *.tar.gz files, and if that fails as well, for *.gz files.
	 * The failed formats will be forwarded to the found plugin to deal with (the gz-reader will have to look for a *.tar reader, which will have to look for *.dcm).
	 */
	static FileFormatList getFileFormatList(std::list<util::istring> format_stack, util::istring dialect);

	/// extract the format stack from a filename
	static std::list<util::istring> getFormatStack( std::string filename );
	/**
	 *  Make images out of a (unordered) list of chunks.
	 *  Uses the chunks in the chunklist to fit them together into images.
	 *  This removes _every_ image from chunks - so make a copy if you need them
	 *  \param chunks list of chunks to be used for the new images.
	 *  \returns a list of newly created images consisting off chunks out of the given chunk list.
	 */
	static std::list<data::Image> chunkListToImageList( std::list<Chunk> &chunks, optional< isis::util::slist& > rejected=optional< isis::util::slist& >() );

	/*
	 * each ImageFormat will be registered in a map after plugin has been loaded
	 * @param plugin pointer to the plugin to register
	 *
	 * @return true if registration was successful, false otherwise
	 * */
	static bool registerFileFormat( const FileFormatPtr plugin, bool front=false );
protected:
	std::list<Chunk> loadPath(const boost::filesystem::path& path, std::list<util::istring> formatstack = {}, util::istring dialect="", optional< util::slist& > rejected=optional< util::slist& >());

	static IOFactory &get();
	IOFactory();//shall not be created directly
	FileFormatList io_formats;

	bool registerFileFormat_impl( const FileFormatPtr plugin, bool front=false );
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
