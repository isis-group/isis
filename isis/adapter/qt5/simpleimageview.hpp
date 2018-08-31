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
#include "../../core/image.hpp"

class QSlider;
class QButtonGroup;

namespace isis{
namespace qt5{
namespace _internal {
class MriGraphicsView;
class TransferFunction{
	data::ValueArrayBase::Converter c;
	std::pair<util::ValueReference,util::ValueReference> minmax;
protected:
	data::scaling_pair scale;
public:
	TransferFunction(std::pair<util::ValueReference,util::ValueReference> in_minmax);
	virtual void operator()(uchar *dst, const data::ValueArrayBase &line)const=0;
	std::pair<double,double> updateScale(qreal bottom, qreal top);
};

}

class SimpleImageView : public QWidget
{
    Q_OBJECT
	QVector<QVector<QPixmap>> slides;
	size_t curr_slice=0,curr_time=0;
	data::Image m_img;
	enum {normal=0,complex,color,mask}type;
	QButtonGroup *transfer_function_group;
	std::shared_ptr<_internal::TransferFunction> transfer_function,magnitude_transfer,phase_transfer;
	
	void setupUi();
	QSlider *sliceSelect,*timeSelect;
	_internal::MriGraphicsView *graphicsView;
protected Q_SLOTS:
	void timeChanged(int time);
	void sliceChanged(int slice);
	void updateImage();
	void selectTransfer(int id, bool checked);
	void reScale(qreal bottom, qreal top);
	void doSave();
public:
    SimpleImageView(data::Image img, QString title="", QWidget *parent=nullptr);
};
}
}

#endif // SIMPLEIMAGEVIEW_HPP
