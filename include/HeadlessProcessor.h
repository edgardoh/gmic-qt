/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file HeadlessProcessor.h
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
#ifndef _GMIC_QT_HEADLESSPROCESSOR_H_
#define _GMIC_QT_HEADLESSPROCESSOR_H_


#include <QTimer>
#include <QObject>
#include <QString>
#include "gmic_qt.h"
#include "gmic_qt_lib.h"

class FilterThread;

namespace cimg_library {
template<typename T> struct CImgList;
}

class HeadlessProcessor : public QObject
{
  Q_OBJECT

public:

  /**
   * @brief Construct a headless processor a given command and arguments
   *        (e.g. GIMP script mode)
   *
   * @param parent
   */
  explicit HeadlessProcessor(QObject *parent,
                             // begin gmic_qt_library
//                             const char * command,
                             const char * arguments,
                             // end gmic_qt_library
                             GmicQt::InputMode inputMode,
                             GmicQt::OutputMode outputMode,
                             // begin gmic_qt_library
                             gmic_filter_execution_data_t * filter_exec_data = nullptr,
                             const char * filterName = nullptr,
                             const char * command = nullptr,
                             const char * previewCommand = nullptr
                             // end gmic_qt_library
														 );

  /**
   * @brief Construct a headless processor using last execution parameters
   *
   * @param parent
   */
  // begin gmic_qt_library
  explicit HeadlessProcessor(QObject *parent = 0, gmic_filter_execution_data_t * filter_exec_data = 0);
  // end gmic_qt_library
  
  ~HeadlessProcessor();
  QString command() const;
  QString filterName() const;
  void setProgressWindowFlag(bool);

public slots:
  void startProcessing();
  void onTimeout();
  void onProcessingFinished();
  void cancel();
signals:
  void singleShotTimeout();
  void done(QString errorMessage);
  void progression(float progress, int duration, unsigned long memory);
private:
  FilterThread * _filterThread;
  cimg_library::CImgList<float> * _gmicImages;
  QTimer _timer;
  QString _filterName;
  QString _lastCommand;
  QString _lastArguments;
  GmicQt::OutputMode _outputMode;
  GmicQt::OutputMessageMode _outputMessageMode;
  GmicQt::InputMode _inputMode;
  QString _lastEnvironment;
  bool _hasProgressWindow;
  QTimer _singleShotTimer;
  // begin gmic_qt_library
  gmic_filter_execution_data_t * _filter_exec_data;
  QString _lastPreviewCommand;
  // end gmic_qt_library
};

#endif // _GMIC_QT_HEADLESSPROCESSOR_H_
