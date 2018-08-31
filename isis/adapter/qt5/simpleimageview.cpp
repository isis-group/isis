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
#include "../../core/io_factory.hpp"
#include <QSlider>
#include <QGridLayout>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QFileDialog>

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

std::pair<double,double> TransferFunction::updateScale(qreal bottom, qreal top){
	const util::Value<double> min = minmax.first->as<double>()+ bottom*(minmax.second->as<double>()-minmax.first->as<double>());
	const util::Value<double> max= minmax.second->as<double>()*top;
	scale= c->getScaling(min,max);
	return std::pair<double,double>(min,max);
}

class MagnitudeTransfer : public TransferFunction{
	template<typename T> void transferFunc(uchar *dst, const data::ValueArray<std::complex<T>> &line)const{
		const T t_scale=scale.first->as<T>();
		const T t_offset=scale.second->as<T>();
		for(const std::complex<T> &v:line){
			*(dst++)=std::min<T>(std::abs(v)*t_scale+t_offset,0xFF);
		}
	}

public:
	MagnitudeTransfer(std::pair<util::ValueReference,util::ValueReference> minmax):TransferFunction(minmax){}
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
	PhaseTransfer():TransferFunction(std::pair<util::ValueReference,util::ValueReference>(util::Value<int16_t>(-180),util::Value<int16_t>(180))){}
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
	LinearTransfer(std::pair<util::ValueReference,util::ValueReference> minmax):TransferFunction(minmax){}
	void operator()(uchar * dst, const data::ValueArrayBase & line) const override{
		if(line.is<util::color24>() || line.is<util::color24>()){
			auto *c_dst=reinterpret_cast<QRgb*>(dst);
			for(const util::color24 &c:const_cast<data::ValueArrayBase &>(line).as<util::color24>(scale))
				*(c_dst++)=qRgb(c.r,c.g,c.b);
		} else 
			line.copyToMem<uint8_t>(dst,line.getLength(),scale);
	}
};

class MaskTransfer : public TransferFunction{
public:
	MaskTransfer():TransferFunction(std::pair<util::ValueReference,util::ValueReference>(util::Value<int>(0),util::Value<int>(1))){}
	void operator()(uchar * dst, const data::ValueArrayBase & line) const override{
		for(const bool &c:const_cast<data::ValueArrayBase &>(line).as<bool>()){
			if(c)
				*(dst++)=255;
			else 
				*(dst++)=0;
		}
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

void SimpleImageView::setupUi(){

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
	
	QPushButton *savebtn=new QPushButton("save",this);
	gridLayout->addWidget(savebtn, 1, 1, 1, 2);
	connect(savebtn, SIGNAL(clicked(bool)), SLOT(doSave()));
	
	connect(timeSelect, SIGNAL(valueChanged(int)), SLOT(timeChanged(int)));
	connect(sliceSelect, SIGNAL(valueChanged(int)), SLOT(sliceChanged(int)));
	
	if(type==complex){
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

	switch(img.getChunkAt(0).getTypeID())
	{
		case data::ValueArray<std::complex<float>>::staticID():
		case data::ValueArray<std::complex<double>>::staticID():
			type=complex;
			break;
		case data::ValueArray<util::color24>::staticID():
		case data::ValueArray<util::color48>::staticID():
			type=color;
		case data::ValueArray<bool>::staticID():
			type=mask;
			break;
		default:
			type=normal;
			break;
	}
	
	setupUi();
	
	if(title.isEmpty())
		title= QString::fromStdString(img.identify(true,false));
	
	if(!title.isEmpty())
		setWindowTitle(title);
	
	const float dpmm_x=physicalDpiX()/25.4, dpmm_y=physicalDpiY()/25.4;
	
	auto voxelsize=img.getValueAs<util::fvector3>("voxelSize");
	graphicsView->scale(voxelsize[0]*dpmm_x,voxelsize[1]*dpmm_y);
	
	auto minmax=img.getMinMax();
	if(type==color){
		auto first_c= minmax.first->as<util::color48>();
		auto second_c=minmax.second->as<util::color48>();
		minmax.first = util::Value<double>((first_c.r+first_c.g+first_c.b)/3.0);
		minmax.second= util::Value<double>((second_c.r+second_c.g+second_c.b)/3.0);
	} else if(type==complex){
		magnitude_transfer.reset(new _internal::MagnitudeTransfer(minmax));
		phase_transfer.reset(new _internal::PhaseTransfer);
		
		transfer_function=magnitude_transfer;
		connect(transfer_function_group, SIGNAL(buttonToggled(int, bool)),SLOT(selectTransfer(int,bool)));
	} else if(type==mask){
		transfer_function.reset(new _internal::MaskTransfer());
	} else {
		transfer_function.reset(new _internal::LinearTransfer(minmax));
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
	
	if(type!=mask){
		QWidget *gradient;
		if(img.hasProperty("window/max") && img.hasProperty("window/min")){
			const double min=minmax.first->as<double>(),max=minmax.second->as<double>();
			const double hmin=img.getValueAs<double>("window/min"),hmax=img.getValueAs<double>("window/max");
			double bottom=(hmin-min) / (max-min);
			double top= hmax / max;

			if(std::isinf(bottom))bottom=0;
			if(std::isinf(top))top=1;

			transfer_function->updateScale(bottom,top);
			
			gradient=new GradientWidget(this,std::make_pair(minmax.first->as<double>(),minmax.second->as<double>()),bottom,top);
		} else 
			gradient=new GradientWidget(this,std::make_pair(minmax.first->as<double>(),minmax.second->as<double>()));
		
		dynamic_cast<QGridLayout*>(layout())->addWidget(gradient,0,2,1,1);
		connect(gradient,SIGNAL(scaleUpdated(qreal, qreal)),SLOT(reScale(qreal,qreal)));
	}

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
	QImage qimage;
	
	const auto ch_dims=m_img.getChunkAt(0,false).getRelevantDims();
	
	assert(ch_dims<=data::sliceDim); // we at least should have a sliced image (the constructor should have made sure of that)
	if(ch_dims==data::sliceDim){ // we have a sliced image
		qimage = makeQImage(
			m_img.getChunk(0,0,curr_slice,curr_time).getValueArrayBase(),
			m_img.getDimSize(data::rowDim),
			[transfer](uchar *dst, const data::ValueArrayBase &line){transfer->operator()(dst,line);}
		);
	} else { // if we have a "lines-image"
		std::vector<data::ValueArrayBase::Reference> lines(m_img.getDimSize(1));
		for(size_t l=0;l<m_img.getDimSize(1);l++){
			lines[l]=m_img.getChunk(0,l,curr_slice,curr_time).getValueArrayBase();
		}
		qimage = makeQImage(
			lines,
			[transfer](uchar *dst, const data::ValueArrayBase &line){transfer->operator()(dst,line);}
		);
	}

	graphicsView->scene()->addPixmap(QPixmap::fromImage(qimage));
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
	std::pair<double,double> window=transfer_function->updateScale(bottom,top);
	m_img.setValueAs("window/min",window.first);
	m_img.setValueAs("window/max",window.second);
	updateImage();
}
void SimpleImageView::doSave(){
	data::IOFactory::write(m_img,QFileDialog::getSaveFileName(this,"Store image as..").toStdString());
}

}
}
