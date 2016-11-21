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
#include "gradientwidget.hpp"
#include "common.hpp"
#include <QSlider>
#include <QGridLayout>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

namespace isis{
namespace qt5{
namespace _internal{
	
TransferFunction::TransferFunction(std::pair<util::ValueReference,util::ValueReference> in_minmax):minmax(in_minmax)
{
	//we dont actually want the converter, but its scaling function
	//which is why we hard-wire it to double->uint8_t, so we allways get a full scaling
	c=data::ValueArrayBase::getConverterFromTo(data::ValueArray<double>::staticID(),data::ValueArray<uint8_t>::staticID());
	updateScale(0,1);
}

void TransferFunction::updateScale(qreal bottom, qreal top){
	const util::Value<double> min = minmax.first->as<double>()+ bottom*(minmax.second->as<double>()-minmax.first->as<double>());
	const util::Value<double> max= minmax.second->as<double>()*top;
	scale= c->getScaling(min,max);
}

class MagnitudeTransfer : public TransferFunction{
	template<typename T> void transferFunc(uchar *dst, const data::ValueArray<std::complex<T>> &line)const{
		const T t_scale=scale.first->as<T>();
		const T t_offset=scale.second->as<T>();
		for(const std::complex<T> &v:line){
			*(dst++)=std::abs(v)*t_scale+t_offset;
		}
	}

public:
	MagnitudeTransfer(const data::Image &img):TransferFunction(img.getMinMax()){}
	void operator()(uchar * dst, const data::ValueArrayBase & line) const override{
		switch(line.getTypeID()){
			case data::ValueArray<std::complex<float>>::staticID():
				transferFunc(dst,line.castToValueArray<std::complex<float>>());
				break;
			case data::ValueArray<std::complex<double>>::staticID():
				transferFunc(dst,line.castToValueArray<std::complex<double>>());
				break;
			default:
				LOG(Runtime,error) << line.getTypeName() << " is no supported complex-type";
		}
	}
};

class PhaseTransfer : public TransferFunction{
	
	template<typename T> void transferFunc(uchar *dst, const data::ValueArray<std::complex<T>> &line)const{
		const T scale=M_PI/128;
		for(const std::complex<T> &v:line){
			*(dst++)=std::arg(v)*scale+128;
		}
	}

public:
	PhaseTransfer(const data::Image &img):TransferFunction(std::pair<util::ValueReference,util::ValueReference>(util::Value<int16_t>(-180),util::Value<int16_t>(180))){}
	void operator()(uchar * dst, const data::ValueArrayBase & line) const override{
		switch(line.getTypeID()){
			case data::ValueArray<std::complex<float>>::staticID():
				transferFunc(dst,line.castToValueArray<std::complex<float>>());
				break;
			case data::ValueArray<std::complex<double>>::staticID():
				transferFunc(dst,line.castToValueArray<std::complex<double>>());
				break;
			default:
				LOG(Runtime,error) << line.getTypeName() << " is no supported complex-type";
		}
	}
};

class LinearTransfer : public TransferFunction{
public:
	LinearTransfer(const data::Image &img):TransferFunction(img.getMinMax()){}
	void operator()(uchar * dst, const data::ValueArrayBase & line) const override{
		line.copyToMem<uint8_t>(dst,line.getLength(),scale);
	}
};

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

} //namespace _internal

void SimpleImageView::setupUi(bool with_complex){

	QGridLayout *gridLayout = new QGridLayout(this);

	sliceSelect = new QSlider(this);
	sliceSelect->setMinimum(1);
	sliceSelect->setOrientation(Qt::Vertical);
	sliceSelect->setTickPosition(QSlider::TicksBelow);

	gridLayout->addWidget(sliceSelect, 0, 1, 1, 1);

    graphicsView = new _internal::MriGraphicsView(this);
	gridLayout->addWidget(graphicsView, 0, 0, 1, 1);

	timeSelect = new QSlider(this);
	timeSelect->setMinimum(1);
	timeSelect->setOrientation(Qt::Horizontal);
	timeSelect->setTickPosition(QSlider::TicksBelow);
	gridLayout->addWidget(timeSelect, 1, 0, 1, 1);
	
	QWidget *gradient=new GradientWidget(this);
	gridLayout->addWidget(gradient,0,2,1,1);
	connect(gradient,SIGNAL(scaleUpdated(qreal, qreal)),SLOT(reScale(qreal,qreal)));
	
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

SimpleImageView::SimpleImageView(data::Image img, QString title, QWidget *parent):QWidget(parent),m_img(img)
{
	if(
		img.getChunkAt(0).getTypeID() == data::ValueArray<std::complex<float>>::staticID() || 
		img.getChunkAt(0).getTypeID() == data::ValueArray<std::complex<double>>::staticID()
	)is_complex=true; //img.getChunkAt(0).getTypeID() is cheaper than Image::getMajorTypeID() and its enough for this case
	else is_complex=false;
	
	setupUi(is_complex);
	
	if(title.isEmpty())
		title= QString::fromStdString(img.identify(true,false));
	
	if(!title.isEmpty())
		setWindowTitle(title);

	if(is_complex){
		magnitude_transfer.reset(new _internal::MagnitudeTransfer(img));
		phase_transfer.reset(new _internal::PhaseTransfer(img));
		
		transfer_function=magnitude_transfer;
		connect(transfer_function_group, SIGNAL(buttonToggled(int, bool)),SLOT(selectTransfer(int,bool)));
	} else {
		transfer_function.reset(new _internal::LinearTransfer(img));;
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

void SimpleImageView::sliceChanged(int slice){
	curr_slice=slice-1;
	updateImage();
}
void SimpleImageView::timeChanged(int time)
{
	curr_time=time-1;
	updateImage();
}
void SimpleImageView::updateImage()
{
	graphicsView->scene()->clear();
	auto transfer=transfer_function; //lambdas cannot bind members ??
	graphicsView->scene()->addPixmap(
		QPixmap::fromImage(
			makeQImage(
				m_img.getChunk(0,0,curr_slice,curr_time).getValueArrayBase(),
				m_img.getDimSize(data::rowDim),
				[transfer](uchar *dst, const data::ValueArrayBase &line){transfer->operator()(dst,line);}
			)
		)
	);
}
void SimpleImageView::selectTransfer(int id, bool checked)
{
	if(checked){//prevent the toggle-off from triggering a useless redraw
		transfer_function= (id==1?magnitude_transfer:phase_transfer);
		updateImage();
	}
}

void SimpleImageView::reScale(qreal bottom, qreal top)
{
	transfer_function->updateScale(bottom,top);
	updateImage();
}


}
}
