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

#include "guiprogressfeedback.hpp"
#include <QProgressBar>
#include <QHBoxLayout>

isis::qt5::GUIProgressFeedback::GUIProgressFeedback(bool autohide, QWidget* parent):QGroupBox(parent),show_always(!autohide),progressbar(new QProgressBar(this))
{
	if(!autohide)
		QWidget::show();
	setLayout(new QHBoxLayout());
	layout()->addWidget(progressbar);
}


void isis::qt5::GUIProgressFeedback::close()
{
	QWidget::hide();
}

size_t isis::qt5::GUIProgressFeedback::extend(size_t by)
{
	progressbar->setMaximum(getMax()+by);
	return getMax();
}

size_t isis::qt5::GUIProgressFeedback::getMax()
{
	return progressbar->maximum();
}

size_t isis::qt5::GUIProgressFeedback::progress(const std::string message, size_t step)
{
	progressbar->setValue(progressbar->value()+step);
	return progressbar->value();
}

void isis::qt5::GUIProgressFeedback::show(size_t max, std::string header)
{
	progressbar->setMaximum(max);
	progressbar->setValue(0);
	QGroupBox::setTitle(QString::fromStdString(header));
	QWidget::show();
}
