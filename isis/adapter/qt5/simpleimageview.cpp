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
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

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

void isis::qt5::SimpleImageView::setupUi(bool with_complex){

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
	
	if(with_complex){
		QGroupBox *groupBox = new QGroupBox("complex representation");
		transfer_function_group = new QButtonGroup(groupBox);
		QHBoxLayout *vbox = new QHBoxLayout;
		groupBox->setLayout(vbox);
		
		QRadioButton *mag=new QRadioButton("magnitude"),*pha=new QRadioButton("phase");
		vbox->addWidget(mag);
		vbox->addWidget(pha);
		transfer_function_group->addButton(mag,1);
		transfer_function_group->addButton(pha,2);
		transfer_function_group->setExclusive(true);
		mag->setChecked(true);
		gridLayout->addWidget(groupBox,2,0,1,1);
	}
}

isis::qt5::SimpleImageView::SimpleImageView(data::Image img, QString title, QWidget *parent):QWidget(parent),m_img(img)
{
	if(
		img.getMajorTypeID() == data::ValueArray<std::complex<float>>::staticID() || 
		img.getMajorTypeID() == data::ValueArray<std::complex<double>>::staticID()
	)is_complex=true;
	else is_complex=false;
	
	setupUi(is_complex);
	
	if(title.isEmpty())
		title= QString::fromStdString(img.identify(true,false));
	
	if(!title.isEmpty())
		setWindowTitle(title);

	if(is_complex){
		const std::pair<util::ValueReference,util::ValueReference> minmax = img.getMinMax();
		const data::ValueArrayBase::Converter &c = data::ValueArrayBase::getConverterFromTo(data::ValueArray<float>::staticID(),data::ValueArray<uint8_t>::staticID());
		const data::scaling_pair magnitude_scale=c->getScaling(*minmax.first,*minmax.second,data::autoscale);
		
		magnitude_transfer = [magnitude_scale](uchar *dst, const data::ValueArrayBase &line){
			const float scale=magnitude_scale.first->as<float>();
			const float offset=magnitude_scale.second->as<float>();
			for(const std::complex<float> &v:line.castToValueArray<std::complex<float>>()){
				*(dst++)=std::abs(v)*scale+offset;
			}
		};

		phase_transfer = [](uchar *dst, const data::ValueArrayBase &line){
			const float scale=M_PI/128;
			for(const std::complex<float> &v:line.castToValueArray<std::complex<float>>()){
				*(dst++)=std::arg(v)*scale+128;
			}
		};
		transfer_function=magnitude_transfer;
		connect(transfer_function_group, SIGNAL(buttonToggled(int, bool)),SLOT(selectTransfer(int,bool)));

	} else {
		const data::scaling_pair scaling=img.getScalingTo(data::ValueArray<uint8_t>::staticID());
		transfer_function=[scaling](uchar *dst, const data::ValueArrayBase &line){
			line.copyToMem<uint8_t>(dst,line.getLength(),scaling);
		};
	}
	
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
			makeQImage(m_img.getChunk(0,0,curr_slice,curr_time).getValueArrayBase(),m_img.getDimSize(data::rowDim),transfer_function)
		)
	);
}
void isis::qt5::SimpleImageView::selectTransfer(int id, bool checked)
{
	if(checked){//prevent the toggle-off from triggering a useless redraw
		transfer_function= (id==1?magnitude_transfer:phase_transfer);
		updateImage();
	}
}

