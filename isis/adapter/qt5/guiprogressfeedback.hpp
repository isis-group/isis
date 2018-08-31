/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <reimer@cbs.mpg.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GUIPROGRESSFEEDBACK_H
#define GUIPROGRESSFEEDBACK_H

#include <QGroupBox>
#include "../../core/progressfeedback.hpp"

class QProgressBar;

namespace isis{
namespace qt5{
	
class GUIProgressFeedback : public QGroupBox, public util::ProgressFeedback
{
    Q_OBJECT
    bool show_always;
	QProgressBar *progressbar;
public:
	GUIProgressFeedback(bool autohide=true,QWidget *parent=nullptr);
	/**
	 * Set the progress display to the given maximum value and "show" it.
	 * This will also extend already displayed progress bars.
	 */
	virtual void show( size_t max, std::string header = "" )override;
	/**
	 * Set the actual "progress".
	 * Behavior is undefined if show was not called before.
	 * \param message message to be displayed (default: "")
	 * \param step increment of the progress (default: 1)
	 * \returns the actual amount of the "progress"
	 */
	virtual size_t progress( const std::string message = "", size_t step = 1 )override;
	///Close/undisplay a progress display.
	virtual void close()override;
	/// \returns the current valued which represents 100%
	virtual size_t getMax()override;
	/// extend the progress bars maximum by the given value
	virtual size_t extend( size_t by )override;
signals:
	void signalNewValue( int value );
	void signalNewMax( int value );
};

}
}

#endif // GUIPROGRESSFEEDBACK_H
