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

#include "simpleimageview.hpp"
#include "common.hpp"
#include <QSlider>
#include <QGridLayout>
#include <QGraphicsView>
#include <QWheelEvent>

class MriGraphicsView: public QGraphicsView{
public:
	MriGraphicsView(QWidget *parent=nullptr):QGraphicsView(parent){
		setDragMode(QGraphicsView::ScrollHandDrag);
	}
	void wheelEvent(QWheelEvent * event) override{
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

		if(event->delta()>0)
			scale(1.1,1.1);
		else
			scale(0.9,0.9);
	}
};

void isis::qt5::SimpleImageView::setupUi(){

	QGridLayout *gridLayout = new QGridLayout(this);

	sliceSelect = new QSlider(this);
	sliceSelect->setMinimum(1);
	sliceSelect->setOrientation(Qt::Vertical);
	sliceSelect->setTickPosition(QSlider::TicksBelow);

	gridLayout->addWidget(sliceSelect, 0, 1, 1, 1);

    graphicsView = new MriGraphicsView(this);
	gridLayout->addWidget(graphicsView, 0, 0, 1, 1);

	timeSelect = new QSlider(this);
	timeSelect->setMinimum(1);
	timeSelect->setOrientation(Qt::Horizontal);
	timeSelect->setTickPosition(QSlider::TicksBelow);
	gridLayout->addWidget(timeSelect, 1, 0, 1, 1);

	connect(timeSelect, SIGNAL(valueChanged(int)), SLOT(timeChanged(int)));
	connect(sliceSelect, SIGNAL(valueChanged(int)), SLOT(sliceChanged(int)));
}

isis::qt5::SimpleImageView::SimpleImageView(data::Image img, QString title, QWidget *parent):QWidget(parent),m_img(img)
{
    setupUi();
	
	if(title.isEmpty())
		title= QString::fromStdString(img.identify(true,false));
	
	if(!title.isEmpty())
		setWindowTitle(title);
	
	scaling=img.getScalingTo(data::ValueArray<uint8_t>::staticID());
	const std::array<size_t,4> img_size= img.getSizeAsVector();
	m_img.spliceDownTo(data::sliceDim);

	if(img_size[data::sliceDim]>1)
		sliceSelect->setMaximum(img_size[data::sliceDim]);
	else
		sliceSelect->setEnabled(false);

	if(img_size[data::timeDim]>1)
		timeSelect->setMaximum(img_size[data::timeDim]);
	else
		timeSelect->setEnabled(false);

	graphicsView->setScene(new QGraphicsScene(0,0,img_size[data::rowDim],img_size[data::columnDim],graphicsView));
	if(img_size[data::sliceDim]>1)
		sliceSelect->setValue(img_size[data::sliceDim]/2);

	updateImage();
}

void isis::qt5::SimpleImageView::sliceChanged(int slice){
	curr_slice=slice-1;
	updateImage();
}
void isis::qt5::SimpleImageView::timeChanged(int time)
{
	curr_time=time-1;
	updateImage();
}
void isis::qt5::SimpleImageView::updateImage()
{
	graphicsView->scene()->clear();
	graphicsView->scene()->addPixmap(
		QPixmap::fromImage(
			makeQImage(m_img.getChunk(0,0,curr_slice,curr_time).getValueArrayBase(),m_img.getDimSize(data::rowDim),scaling)
		)
	);
}

