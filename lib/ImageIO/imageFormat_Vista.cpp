/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive
 * and Brain Sciences, Leipzig.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Thomas Pröger, proeger@cbs.mpg.de, 2010
 *
 *****************************************************************/

#include "imageFormat_Vista.h"

namespace isis
{

namespace image_io
{

ImageFormat_Vista::ImageFormat_Vista()
{
	// TODO Auto-generated constructor stub
}

ImageFormat_Vista::~ImageFormat_Vista()
{
	// TODO Auto-generated destructor stub
}

bool ImageFormat_Vista::write( const data::Image &image, const std::string &filename, const std::string &dialect )
{
}

int ImageFormat_Vista::load( data::ChunkList &chunks, const std::string &filename, const std::string &dialect )
{
}

}
}

isis::image_io::FileFormat* factory()
{
	return new isis::image_io::ImageFormat_Vista();
}
