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

#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H

#include <QWidget>

class GradientWidget : public QWidget
{
    Q_OBJECT
    QLinearGradient shade;
	qreal bottom,top;
	std::pair< double, double > image_rage;
	const QString min_str,max_str;

	bool adaptWidth(QPainter* p, QString text, int offset);
protected:
	QImage generateShade();
	void paintEvent(QPaintEvent * event) override;
	void mouseMoveEvent(QMouseEvent * ) override;
public:
	GradientWidget(QWidget* parent, std::pair< double, double > in_image_rage, qreal bottom = 0, qreal top = 1);
signals:
	void scaleUpdated(qreal bottom, qreal top);
};

#endif // GRADIENTWIDGET_H
