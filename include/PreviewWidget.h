/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file PreviewWidget.h
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
#ifndef _GMIC_QT_PREVIEWWIDGET_H_
#define _GMIC_QT_PREVIEWWIDGET_H_

#include <QWidget>
#include <QPixmap>
#include <QSize>
#include <QRect>
#include <QImage>
#include <QMutex>
#include "host.h"

namespace cimg_library {
 template<typename T> struct CImgList;
}

class PreviewWidget : public QWidget
{
  Q_OBJECT

public:
  explicit PreviewWidget(QWidget *parent = 0);
  ~PreviewWidget();
  void setFullImageSize(const QSize & );
  void normalizedVisibleRect(double & x, double & y, double & width, double & height) const;
  bool isAtDefaultZoom() const;
  bool isAtFullZoom() const;
  void updateImageNames( cimg_library::CImgList<char> & imageNames, GmicQt::InputMode mode);
  double currentZoomFactor() const;
  void updateVisibleRect();
  void centerVisibleRect();
  // begin gmic_qt_library
  void setPreviewImage(const QImage & image, gmic_filter_execution_data_t * filter_data);
  // end gmic_qt_library
  const QImage & image() const;
  void translateNormalized(double dx, double dy );
  void translateFullImage(double dx, double dy);

protected:
  void resizeEvent (QResizeEvent *) override;
  void timerEvent(QTimerEvent*) override;
  bool event(QEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;
  void paintEvent(QPaintEvent *e) override;
  bool eventFilter(QObject *, QEvent *event) override;

signals:
  void previewUpdateRequested();
  void zoomChanged(double zoom);

public slots:
  void abortUpdateTimer();
  void sendUpdateRequest();
  void onMouseTranslationInImage(QPoint shift);
  void zoomIn();
  void zoomOut();
  void zoomFullImage();
  void zoomIn(QPoint, int steps);
  void zoomOut(QPoint, int steps);
  void setZoomLevel(double zoom);
  /**
   * @brief setPreviewFactor
   * @param filterFactor
   * @param reset If true, zoomFactor is set to "Full Image" if
   *              filterFactor is GmicQt::PreviewFactorAny
   */
  void setPreviewFactor(float filterFactor, bool reset);
  void displayOriginalImage();
  QImage originalImage();
  void onPreviewParametersChanged();
  void enablePreview(bool);
  void invalidateSavedPreview();
  void savePreview();
  void restorePreview();

private:
  double defaultZoomFactor() const;
  QImage _image;
  QImage _savedPreview;
  QSize _fullImageSize;
  double _currentZoomFactor;

  /*
   * (0) for a 1:1 preview
   * (1) for previewing the whole image
   * (2) for 1/2 image
   * GmigQt::PreviewFactorAny
   */
  float _previewFactor;
  int _timerID;
  bool _previewEnabled;

  struct PreviewPosition {
    double x;
    double y;
    double w;
    double h;
    bool operator!=(const PreviewPosition & ) const;
    bool operator==(const PreviewPosition & ) const;
    bool isFull() const;
    static const PreviewPosition Full;
  };

  static const int RESIZE_DELAY = 400;

  PreviewPosition _visibleRect;

  bool _pendingResize;
  QPixmap _transparency;
  bool _savedPreviewIsValid;
  QRect _imagePosition;
  QPoint _mousePosition;
  QPixmap _transparentBackground;
  bool _paintOriginalImage;
  QSize _originaImageSize;
  QSize _originaImageScaledSize;

  QImage _cachedOriginalImage;
  PreviewPosition _cachedOriginalImagePosition;
  PreviewPosition _positionAtUpdateRequest;
  // begin gmic_qt_library
  gmic_filter_execution_data_t * _filter_exec_data;
  // end gmic_qt_library
};

#endif // _GMIC_QT_PREVIEWWIDGET_H_
