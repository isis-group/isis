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

#ifndef SORTEDCHUNKLIST_HPP
#define SORTEDCHUNKLIST_HPP

#include "chunk.hpp"
#include "CoreUtils/vector.hpp"
#include <stack>
#include <boost/shared_ptr.hpp>

namespace isis{
namespace data{
namespace _internal{

struct sortComparator{
	sortComparator(const std::string& prop_name);
	std::string propertyName;
	virtual bool operator() ( const Chunk &a, const Chunk &b )const=0;
};
struct originComparator:sortComparator{
	originComparator(const std::string& prop_name);
	bool operator() ( const Chunk &a, const Chunk &b )const;
};
struct timeComparator:sortComparator{
	timeComparator(const std::string& prop_name);
	bool operator() ( const Chunk &a, const Chunk &b )const;
};


class SortedChunkList:std::map<util::PropertyValue,std::map<util::PropertyValue,Chunk,sortComparator>,sortComparator>
{
protected:
	std::stack<boost::shared_ptr<sortComparator> > secondarySort;
	boost::shared_ptr<sortComparator> primarySort;
	SortedChunkList();
};

}}}

#endif // SORTEDCHUNKLIST_HPP
