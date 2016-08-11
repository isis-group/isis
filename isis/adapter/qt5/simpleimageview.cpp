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
#include "ui_simpleimageview.h"


isis::qt5::SimpleImageView::SimpleImageView(data::TypedImage<uint8_t>::Image img, QWidget *parent):QWidget(parent),ui(new Ui_SimpleImageView)
{
    ui->setupUi(this);

	const std::array<size_t,4> img_size= img.getSizeAsVector();




	img.spliceDownTo(data::sliceDim);
	slides.resize(img_size[data::timeDim]);
	for(int t=0;t<img_size[data::timeDim];t++){
		slides[t].resize(img_size[data::sliceDim]);
		for(int s=0;s<img_size[data::sliceDim];s++){

			QImage qi(img_size[data::rowDim],img_size[data::columnDim], QImage::Format_Grayscale8);
			int idx=0;
			for(const uint8_t &v:img.getChunkAs<uint8_t>(0,0,s,t)) {
				qi.setPixel(idx%img_size[data::rowDim],idx/img_size[data::rowDim], qRgb(v,v,v));
				++idx;
			}
			slides[t][s]=QPixmap::fromImage(qi);
		}
	}
	if(img_size[data::sliceDim]>1)
		ui->sliceSelect->setMaximum(img_size[data::sliceDim]);
	else
		ui->sliceSelect->setEnabled(false);

	if(img_size[data::timeDim]>1)
		ui->timeSelect->setMaximum(img_size[data::timeDim]);
	else
		ui->timeSelect->setEnabled(false);

	ui->graphicsView->setScene(new QGraphicsScene(0,0,img_size[data::rowDim],img_size[data::columnDim],ui->graphicsView));
}
isis::qt5::SimpleImageView::~SimpleImageView()
{
	delete ui;
}

void isis::qt5::SimpleImageView::sliceChanged(int slice)
{
	curr_slice=slice-1;
	ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(slides[curr_time][curr_slice]);
}
void isis::qt5::SimpleImageView::timeChanged(int time)
{
	curr_time=time-1;
	ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(slides[curr_time][curr_slice]);
}
