/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  Enrico Reimer <reimer@cbs.mpg.de>
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

#ifndef SIMPLEIMAGEVIEW_HPP
#define SIMPLEIMAGEVIEW_HPP

#include <QWidget>
#include <QList>
#include <QPixmap>
#include "../../data/image.hpp"

class Ui_SimpleImageView;

namespace isis{
namespace qt5{

class SimpleImageView : public QWidget
{
    Q_OBJECT
	QVector<QVector<QPixmap>> slides;
	size_t curr_slice=0,curr_time=0;
	data::Image m_img;
	data::scaling_pair scaling;
protected Q_SLOTS:
	void timeChanged(int time);
	void sliceChanged(int slice);
	void updateImage();
public:
    SimpleImageView(data::Image img, QWidget *parent=nullptr);
	~SimpleImageView();

private:
    Ui_SimpleImageView* ui;
};
}
}

#endif // SIMPLEIMAGEVIEW_HPP
