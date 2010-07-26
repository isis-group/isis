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

#include "progressfeedback.hpp"
#include "common.hpp"

namespace isis{ namespace util{

ProgressFeedback &ProgressFeedback::operator++()
{
	progress();
	return *this;
}

void ConsoleFeedback::show(size_t max, std::string header)
{
	header+="\n";
	disp.reset(new boost::progress_display(max,std::cout,header));
}
void ConsoleFeedback::close()
{
	disp.reset();
}
size_t ConsoleFeedback::getMax()
{
	return disp ? disp->expected_count():0;
}
size_t ConsoleFeedback::progress(const std::string message, size_t step)
{
	LOG_IF(!disp,Debug,error) << "Cannot call progress on a not displayed ConsoleFeedBack.";
	LOG_IF(!message.empty(),Debug,warning) << "ConsoleFeedBack does ignore the message string";
	disp->operator+=(step);
}


}}