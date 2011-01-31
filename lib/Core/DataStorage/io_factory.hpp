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
	typedef boost::shared_ptr< ::isis::image_io::FileFormat> FileFormatPtr;
	typedef std::list<FileFormatPtr> FileFormatList;
private:
	util::ProgressFeedback *m_feedback;
public:
	/**
	 * load a data file with given filename and dialect
	 * @param ret ChunkList to store the loaded chunks in
	 * @param filename file to open
	 * @param suffix_override override the given suffix with this one (especially if there's no suffix)
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	int loadFile( std::list<Chunk> &ret, const boost::filesystem::path &filename, std::string suffix_override, std::string dialect );
	int loadPath( std::list<Chunk> &ret, const boost::filesystem::path &path, std::string suffix_override, std::string dialect );
	static std::list<data::Image> load( const std::string &path, std::string suffix_override = "", std::string dialect = "" );

	static bool write( std::list<data::Image> images, const std::string &path, std::string suffix_override, const std::string &dialect );

	static FileFormatList getFormats();
	static IOFactory &get();

	static void setProgressFeedback( util::ProgressFeedback *feedback );
	FileFormatList getFormatInterface( std::string filename, std::string suffix_override = "", std::string dialect = "" );
protected:
	IOFactory();//shall not be created directly
	FileFormatList io_formats;
	/**
	 *  Make images out of a (unordered) list of chunks.
	 *  Uses the chunks in the chunklist to fit them together into images.
	 *  \param chunk list of chunks to be used for the new images.
	 *  \returns a list of newly created images consisting off chunks out of the given chunk list.
	 */
	static std::list<data::Image> chunkListToImageList(std::list<Chunk> &chunks);

	/*
	 * each ImageFormat will be registered in a map after plugin has been loaded
	 * @param plugin pointer to the plugin to register
	 *
	 * @return true if registration was successful, false otherwise
	 * */
	bool registerFormat( const FileFormatPtr plugin );
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
