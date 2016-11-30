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
#include <cmath>

GradientWidget::GradientWidget(QWidget* parent, std::pair< double, double > in_image_rage, qreal in_bottom, qreal in_top):
bottom(1-in_bottom),top(1-in_top),image_rage(in_image_rage),min_str(QString::number(in_image_rage.first,'f',4)),max_str(QString::number(in_image_rage.second,'f',4))
{
	setMinimumWidth(30);
	setMaximumWidth(30);
	setMouseTracking(true);
	if(bottom>1)bottom=1;
	if(top<0)top=0;
}

bool GradientWidget::adaptWidth(QPainter *p,QString text, int offset)
{
	const int num_width=p->boundingRect(QRect(),Qt::AlignLeft|Qt::TextSingleLine,text).width();
	if(num_width+ 30 >= width()){
		setMinimumWidth(num_width+ offset+1);
		setMaximumWidth(num_width+ offset+1);
		return true;
	} else
		return false;
}


void GradientWidget::paintEvent(QPaintEvent* event)
{
	QPainter p(this);
	
	const int text_hight=p.boundingRect(QRect(),Qt::AlignLeft|Qt::TextSingleLine,"0").height();
	
	if(adaptWidth(&p,min_str,30) || adaptWidth(&p,max_str,30))
		return;
	
	p.eraseRect(15,0,width()-15,height());
	
	for(unsigned int i = 0; i < 10; i++)
		p.drawLine(15,i*height()/10,25,i*height()/10);

	if(height()>250)
		for(unsigned int i = 0; i < 50; i++)
			p.drawLine(15,i*height()/50,20,i*height()/50);

	const int top_pos=top*height();
	const int bottom_pos=bottom*height();

	p.drawImage(0, 0, generateShade());

	p.setPen(Qt::blue);
	p.drawRect(0, 0, 15 - 1, height() - 1);

	p.setBrush(QBrush(QColor(0, 0, 191, 50)));

	p.setPen(QPen(QColor(255, 255, 255, 191), 1));
	p.drawRect(QRect(0,bottom_pos-3,25,6));

	p.setPen(QPen(QColor(0, 0, 0, 191), 1));
	p.drawRect(QRect(0,top_pos-3,25,6));
	
	if(top_pos>text_hight && top_pos<height()-text_hight){
		QString number=QString::number(image_rage.first+(1-top)*(image_rage.second-image_rage.first),'f',4);
		if(adaptWidth(&p,number,30))
			return;
		p.drawText(QPoint(30,top_pos+text_hight/2-3),number);
	}
	
	if(bottom_pos<height()-text_hight && bottom_pos>text_hight){
		QString number=QString::number(image_rage.first+(1-bottom)*(image_rage.second-image_rage.first),'f',4);
		if(adaptWidth(&p,number,30))
			return;
		p.drawText(QPoint(30,bottom_pos+text_hight/2-3),number);
	}
	
	p.drawText(QPoint(30,text_hight-6),max_str);
	p.drawText(QPoint(30,height()),min_str);
}


QImage GradientWidget::generateShade()
{
	shade = QLinearGradient(0, 0, 0, height());
	shade.setColorAt(bottom, Qt::black);
	shade.setColorAt(top, Qt::white);

	QImage ret(QSize(15,height()), QImage::Format_Grayscale8);
	QPainter p(&ret);
	p.fillRect(QRect(0,0,15,height()), shade);
	return ret;
}

void GradientWidget::mouseMoveEvent(QMouseEvent *event)
{
	const qreal pos=event->pos().y();
	
	if(pos<0 || pos > height())
		return;
	
	const int bottom_dist=std::abs(bottom*height()-pos);
	const int top_dist=   std::abs(   top*height()-pos);
	
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

