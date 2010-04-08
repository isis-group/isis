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

#include "CoreUtils/application.hpp"
#include "DataStorage/image.hpp"

namespace isis{ namespace data {

class IOApplication:public util::Application
{
	bool m_input,m_output;
public:
	data::ImageList images;
	IOApplication(const char name[],bool have_input=true,bool have_output=true);
	virtual bool init(int argc, char** argv, bool exitOnError = true);
};
	
}}

#endif // DATA_IOAPPLICATION_HPP
