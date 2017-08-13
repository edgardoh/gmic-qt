/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file IntParameter.h
 *
 *  Copyright 2017 Sebastien Fourey
 *
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  gmic_qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gmic_qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gmic_qt.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _GMIC_QT_INTPARAMETER_H_
#define _GMIC_QT_INTPARAMETER_H_

#include "AbstractParameter.h"
#include <QString>
class QSpinBox;
class QSlider;
class QLabel;

class IntParameter : public AbstractParameter {
  Q_OBJECT

public:
  IntParameter(QObject * parent = 0);
  ~IntParameter();
  void addTo(QWidget *, int row);
  QString textValue() const;
  void setValue(const QString & value);
  void reset();
  void initFromText(const char * text, int & textLength);

protected:
  void timerEvent(QTimerEvent*);

public slots:
  void onSliderMoved(int);
  void onSliderValueChanged(int value);
  void onSpinBoxChanged(int);

signals:
  void valueChanged();

private:
  void connectSliderSpinBox();
  void disconnectSliderSpinBox();
//  QString _name;
  int _min;
  int _max;
  int _default;
  int _value;
  QLabel * _label;
  QSlider * _slider;
  QSpinBox * _spinBox;
  int _timerId;
  static const int UPDATE_DELAY = 300;
  bool _connected;
};

#endif // _GMIC_QT_INTPARAMETER_H_
