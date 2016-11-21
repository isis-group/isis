/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
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

#include "gradientwidget.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <iostream>

GradientWidget::GradientWidget(QWidget* parent,uint8_t min,uint8_t max):QWidget(parent),shade(0, min, 0, 255)
{
	setMinimumWidth(15);
	setMaximumWidth(15);
	setMouseTracking(true);
}

void GradientWidget::paintEvent(QPaintEvent* event)
{
	QPainter p(this);
	p.drawImage(0, 0, generateShade());

	p.setPen(Qt::blue);
	p.drawRect(0, 0, width() - 1, height() - 1);

	p.setBrush(QBrush(QColor(0, 0, 191, 50)));

	p.setPen(QPen(QColor(255, 255, 255, 191), 1));
	p.drawRect(QRect(0,bottom*height()-3,width(),6));
	
	
	
	p.setPen(QPen(QColor(0, 0, 0, 191), 1));
	p.drawRect(QRect(0,top*height()-3,width(),6));
}


QImage GradientWidget::generateShade()
{
	shade = QLinearGradient(0, 0, 0, height());
	shade.setColorAt(bottom, Qt::black);
	shade.setColorAt(top, Qt::white);

	QImage ret(size(), QImage::Format_Grayscale8);
	QPainter p(&ret);
	p.fillRect(rect(), shade);
	return ret;
}

void GradientWidget::mouseMoveEvent(QMouseEvent *event)
{
	const qreal pos=event->pos().y();
	
	if(pos<0 || pos > height())
		return;
	
	const int bottom_dist=abs(bottom*height()-pos);
	const int top_dist=   abs(   top*height()-pos);
	
	if( bottom_dist< 6 ) {
		setCursor(Qt::PointingHandCursor);
		if(bottom_dist!=0 && event->buttons()&Qt::LeftButton){
			bottom= pos/height();
			if(bottom<top)top=bottom;
			update();
			emit scaleUpdated(1.-bottom, 1.-top);
		}
	} else if(top_dist < 6){
		setCursor(Qt::PointingHandCursor);
		if(top_dist!=0 && event->buttons()&Qt::LeftButton){
			top= pos/height();
			if(top>bottom)bottom=top;
			update();
			emit scaleUpdated(1.-bottom, 1.-top);
		}
	} else
		unsetCursor();
}

