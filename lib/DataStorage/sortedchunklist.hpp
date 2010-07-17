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

class SortedChunkList
{
public:
	struct sortComparator{
		sortComparator(const std::string& prop_name);
		std::string propertyName;
		virtual bool operator() ( const util::PropertyValue &a, const util::PropertyValue &b )const=0;
		virtual ~sortComparator();
	};
	struct scalarPropCompare:public sortComparator{
		scalarPropCompare(const std::string& prop_name);
        bool operator()(const isis::util::PropertyValue& a, const isis::util::PropertyValue& b) const;
	};
	struct fvectorCompare:public sortComparator{
		fvectorCompare(const std::string& prop_name);
        bool operator()(const isis::util::PropertyValue& a, const isis::util::PropertyValue& b) const;
	};
private:
	typedef std::map<util::PropertyValue,boost::shared_ptr<Chunk>,scalarPropCompare> SecondaryMap;
	typedef std::map<util::fvector4,SecondaryMap,fvectorCompare> PrimaryMap;
	
	std::stack<scalarPropCompare> secondarySort;
	PrimaryMap chunks;

	// low level finding
	boost::shared_ptr<Chunk> secondaryFind(const util::PropertyValue &key, SecondaryMap& map);
	SecondaryMap *primaryFind(const util::fvector4& key);

	// low level inserting
	std::pair<boost::shared_ptr<Chunk>,bool> secondaryInsert(SecondaryMap &map,const Chunk &ch);
	std::pair<boost::shared_ptr<Chunk>,bool> primaryInsert(const Chunk &ch);

	std::list<std::string> equalProps;
public:
	SortedChunkList(const std::string fvectorPropName,std::string comma_separated_equal_props);
	
	std::string getPrimarySortPropertyName()const;
	std::string getSecondarySortPropertyName()const;

	void addSecondarySort(const std::string &cmp);
	bool popSecondarySort();
	
	bool insert(const Chunk &ch);
	bool empty()const;
	void clear();
	
	std::vector<boost::weak_ptr<Chunk> > getLookup();
	bool isRectangular();
};


}}}

#endif // SORTEDCHUNKLIST_HPP
