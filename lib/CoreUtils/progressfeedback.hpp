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

#ifndef PROGRESSFEEDBACK_HPP
#define PROGRESSFEEDBACK_HPP
#include <stddef.h>
#include <string>
#include <boost/progress.hpp>
#include <boost/scoped_ptr.hpp>

namespace isis{ namespace util{

class ProgressFeedback
{
public:
	/**
	 * (Re)Set the progress display to the given maximum value and "show" it.
	 * This will also reset the progress to 0.
	 */
	virtual void show(size_t max,std::string header="")=0;
	/**
	 * Increment the "progress" by step "steps".
	 * Behavior is undefined if show was not called before.
	 */
	virtual size_t progress(const std::string message="",size_t step=1)=0;
	///Close/undisplay a progress display.
	virtual void close()=0;
	/// get the current valued which represents 100%
	virtual size_t getMax()=0;
	/// Increment the "progress" by one
	ProgressFeedback& operator++();
};

class ConsoleFeedback:public ProgressFeedback{
	boost::scoped_ptr<boost::progress_display> disp;
public:
    void close();
    size_t getMax();
    size_t progress(const std::string message = "", size_t step = 1);
    void show(size_t max, std::string header);
};
}}
#endif // PROGRESSFEEDBACK_HPP
