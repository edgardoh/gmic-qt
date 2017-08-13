/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file LayersExtendsProxy.cpp
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
#include "LayersExtentProxy.h"
#include "host.h"

int LayersExtentProxy::_width = -1;
int LayersExtentProxy::_height = -1;
GmicQt::InputMode LayersExtentProxy::_inputMode = GmicQt::All;
// begin gmic_qt_library
QSize LayersExtentProxy::getExtent(GmicQt::InputMode mode, gmic_filter_execution_data_t * filter_exec_data)
// end gmic_qt_library
{
  QSize size;
  // begin gmic_qt_library
  getExtent(mode,size.rwidth(),size.rheight(),filter_exec_data);
  // end gmic_qt_library
  return size;
}
// begin gmic_qt_library
void LayersExtentProxy::getExtent(GmicQt::InputMode mode, int & width, int & height, gmic_filter_execution_data_t * filter_exec_data)
// end gmic_qt_library
{
  if ( mode != _inputMode || _width == -1 || _height == -1 ) {
    // begin gmic_qt_library
    gmic_qt_get_layers_extent(&_width,&_height,mode,filter_exec_data);
    // end gmic_qt_library
    width = _width;
    height = _height;
  } else {
    width = _width;
    height = _height;
  }
  _inputMode = mode;
}

void LayersExtentProxy::clearCache()
{
  _width = _height = -1;
}
