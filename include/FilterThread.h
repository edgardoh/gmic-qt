/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file FilterThread.h
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
#ifndef _GMIC_QT__FILTERTHREAD_H_
#define _GMIC_QT__FILTERTHREAD_H_

#include <QThread>
#include <QTime>

class ImageSource;
class QMutex;
class QImage;
class QSemaphore;
#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QMutex>
#include <QPainter>
#include <QImage>
#include <QTime>
#include <QFile>

#include "Common.h"
#include "gmic_qt.h"
#include "host.h"

namespace cimg_library {
template<typename T> struct CImgList;
}

class FilterThread : public QThread {
  Q_OBJECT

public:
  FilterThread(QObject * parent,
               const QString & name,
               const QString & command,
               // begin gmic_qt_library
	       const QString & originalName,
               const QString & previewCommand,
               // end gmic_qt_library
               const QString & arguments,
               // begin gmic_qt_library
               const QList<QString> & paramsValues,
               bool runPreview,
		const float imageScale,
               // end gmic_qt_library
               const QString & environment,
               GmicQt::OutputMessageMode mode);

  virtual ~FilterThread();
  void run();
  void setArguments(const QString &);
  void setInputImages( const cimg_library::CImgList<float> & list,
                       const cimg_library::CImgList<char> & imageNames );
  const cimg_library::CImgList<float> & images() const;
  const cimg_library::CImgList<char> & imageNames() const;
  QString gmicStatus() const;
  QString errorMessage() const;
  bool failed() const;
  bool aborted() const;
  int duration() const;
  float progress() const;
  QString name() const;
  // begin gmic_qt_library
  QString command() const;
  QString originalName() const;
  QString previewCommand() const;
  QString arguments() const;
  QList<QString> paramsValues() const;
  // end gmic_qt_library
  QString fullCommand() const;

public slots:
  void abortGmic();

signals:
  void done();

private:
  void setCommand(const QString & command);
  QString _command;
  // begin gmic_qt_library
  QString _originalName;
  QString _previewCommand;
  // end gmic_qt_library
  QString _arguments;
  // begin gmic_qt_library
  QList<QString> _paramsValues;
  bool _runPreview;
  float _imageScale;
  // end gmic_qt_library
  QString _environment;
  cimg_library::CImgList<float> * _images;
  cimg_library::CImgList<char> * _imageNames;
  bool _gmicAbort;
  bool _failed;
  QString _gmicStatus;
  float _gmicProgress;
  QString _errorMessage;
  QString _name;
  GmicQt::OutputMessageMode _messageMode;
  QTime _startTime;
};

#endif // _GMIC_QT__FILTERTHREAD_H_
