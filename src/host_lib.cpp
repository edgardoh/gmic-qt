/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file host_gimp.cpp
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
/*
#include <QDebug>
#include <QFileInfo>
#include <QRegExp>
#include <algorithm>
#include <limits>
#include "host.h"
#include "ImageTools.h"
#include "Common.h"
#include "GmicStdlibParser.h"
#include "gmic.h"
*/

#include <cstring>
#include "GmicLibParser.h"
#include "host.h"
#include "gmic.h"

namespace GmicQt {
const QString HostApplicationName = QString("LIB");
const char * HostApplicationShortname = GMIC_QT_XSTRINGIFY(GMIC_HOST);
}
/*
char * buildOutputParamsFromList(QList<QString>& outputParamsList)
{
	char * outputParams = 0;
	QString str = GmicLibParser::buildArgumentsFromParamsList(outputParamsList);
	
	if (str.size() > 0) {
		outputParams = (char*)std::calloc(1, (str.size()+1)*sizeof(char));
	}
	// copy the parameters
	if (outputParams) {
		std::strcpy(outputParams, str.toLatin1().constData());
	}
	
	return outputParams;
}
*/
/*
char * buildFilterData1(const char * filterName, const char * filterCommand, const char * filterPreviewCommand, 
		QList<QString>& paramsList, int& filter_data_size)
{
	char * outputParams = 0;
	filter_data_size = 0;
	
	// calculate params size
	for (int i = 0; i < paramsList.size(); i++) {
		filter_data_size += paramsList[i].size() + 1;
	}
	if (filterName)
		filter_data_size += std::strlen(filterName) + 1;
	else
		filter_data_size++;
	if (filterCommand)
		filter_data_size += std::strlen(filterCommand) + 1;
	else
		filter_data_size++;
	if (filterPreviewCommand)
		filter_data_size += std::strlen(filterPreviewCommand) + 1;
	else
		filter_data_size++;
	
	// alloc space for the parameters
	if (filter_data_size > 0) {
		outputParams = (char*)std::calloc(1, filter_data_size*sizeof(char));
	}
	// copy <filter name>\0<commnad>\0<preview command>\0<parameters values> parameters
	if (outputParams) {
		char *params = outputParams;
		
		if (filterName) {
			std::strcpy(params, filterName);
			params += std::strlen(filterName) + 1;
		} else {
			params++;
		}
		
		if (filterCommand) {
			std::strcpy(params, filterCommand);
			params += std::strlen(filterCommand) + 1;
		} else {
			params++;
		}
		
		if (filterPreviewCommand) {
			std::strcpy(params, filterPreviewCommand);
			params += std::strlen(filterPreviewCommand) + 1;
		} else {
			params++;
		}
		
		for (int i = 0; i < paramsList.size(); i++) {
			std::strcpy(params, paramsList[i].toLatin1().constData());
			
			params += paramsList[i].size() + 1;
		}
	}
	
	return outputParams;
}
*/

void gmic_qt_show_message(const char * /* message*/ )
{

}

void gmic_qt_apply_color_profile(cimg_library::CImg<float> & /*image*/,gmic_filter_execution_data_t * /*filter_exec_data*/)
{

}

void gmic_qt_get_layers_extent(int * width, int * height, GmicQt::InputMode /* mode*/,gmic_filter_execution_data_t * filter_exec_data)
{
  if (filter_exec_data) {
    *width = filter_exec_data->width;
    *height = filter_exec_data->height;
  }
  else {
    *width = 0;
    *height = 0;
  }
}

void gmic_qt_get_image_size(int * width, int * height, gmic_filter_execution_data_t * filter_exec_data)
{
  if (filter_exec_data) {
    *width = filter_exec_data->width;
    *height = filter_exec_data->height;
  }
  else
  {
    *width = 0;
    *height = 0;
  }
}

void gmic_qt_get_cropped_images( gmic_list<float> & images,
                                 gmic_list<char> & imageNames,
                                 double  x,
                                 double  y,
                                 double width,
                                 double height,
                                 GmicQt::InputMode mode,
																 gmic_filter_execution_data_t * filter_exec_data )
{
  images.assign();
  imageNames.assign();
  if (filter_exec_data == 0 || mode == GmicQt::NoInput) return;
  
  const bool entireImage = x < 0 && y < 0 && width < 0 && height < 0;
  if (entireImage) {
    x = 0.0;
    y = 0.0;
    width = 1.0;
    height = 1.0;
  }
  
  float *f_image = filter_exec_data->image;
  const int ix = static_cast<int>(entireImage?0:std::floor(x * filter_exec_data->width));
  const int iy = static_cast<int>(entireImage?0:std::floor(y * filter_exec_data->height));
  const int iw = entireImage?filter_exec_data->width:std::min(filter_exec_data->width-ix,static_cast<int>(1+std::ceil(width * filter_exec_data->width)));
  const int ih = entireImage?filter_exec_data->height:std::min(filter_exec_data->height-iy,static_cast<int>(1+std::ceil(height * filter_exec_data->height)));

  if (f_image != NULL && iw > 0 && ih > 0)
  {
    images.assign(1);
    imageNames.assign(1);

    const int f_spectrum = filter_exec_data->spectrum;
    const int f_width = filter_exec_data->width;
    const int f_ch = filter_exec_data->spectrum;

    cimg_library::CImg<gmic_pixel_type> &img = images[0];
    img.assign(iw, ih, 1, f_spectrum);
    
    float *ptr_image = img._data;
    for (int c = 0; c<f_spectrum; ++c)
    {
      for (int y = 0; y<ih; ++y)
      {
        float *ptr2 = ptr_image + (img.width() * img.height() * c) + (img.width() * y);
        
        for (int x = 0; x<iw; ++x)
        {
          *(ptr2++) = f_image[((y+iy)*f_width*f_ch)+((x+ix)*f_ch)+c];
        }
      }
    }

    QString str((const char*)"Image 0");
    gmic_image<char> & name = imageNames[0];
    name.resize(str.size()+1);
    std::memcpy(name.data(),str.toLatin1().constData(),name.width());
  }
  
}

void gmic_qt_output_images( gmic_list<gmic_pixel_type> & images,
                            const gmic_list<char> & imageNames,
                            GmicQt::OutputMode /*outputMode*/,
														const QString& filterName, 
														const QString& filterCommand,
														const QString& filterPreviewCommand,
														const QList<QString>& paramsValues,
														gmic_filter_execution_data_t * filter_exec_data,
														const char * /*verboseLayersLabel*/ )
{
  Q_ASSERT(images.size() == imageNames.size());
  
  if (images.size() > 0 && filter_exec_data)
  {
    float *f_image = filter_exec_data->image;
    cimg_library::CImg<gmic_pixel_type> &img = images[0];
    
    const int iw = std::min(img.width(), filter_exec_data->width);
    const int ih = std::min(img.height(), filter_exec_data->height);
    const int is = std::min(img.spectrum(), filter_exec_data->spectrum);

//    if (f_image) std::memset(f_image, 0, filter_exec_data->width * filter_exec_data->height * filter_exec_data->spectrum * sizeof(float));
    
    if (f_image != NULL && iw > 0 && ih > 0)
    {
      const int f_width = filter_exec_data->width;
      const int f_ch = filter_exec_data->spectrum;

      float *ptr_image = img._data;

      for (int c = 0; c<is; ++c)
      {
        for (int y = 0; y<ih; ++y)
        {
          float *ptr2 = ptr_image + (img.width() * img.height() * c) + (img.width() * y);
          
          for (int x = 0; x<iw; ++x)
          {
            f_image[(y*f_width*f_ch)+(x*f_ch)+c] = *(ptr2++);
          }
        }
      }
      
      filter_exec_data->return_width = img.width();
      filter_exec_data->return_height = img.height();
      filter_exec_data->return_spectrum = img.spectrum();
      filter_exec_data->return_value = 1;
      
/*      if (filterName) {
        if (filter_exec_data->filter_name) std::free(filter_exec_data->filter_name);
        filter_exec_data->filter_name = (char*)std::calloc(1, (std::strlen(filterName)+1)*sizeof(char));
        std::strcpy(filter_exec_data->filter_name, filterName);
      }
      
      if (filterCommand) {
        if (filter_exec_data->filter_command) std::free(filter_exec_data->filter_command);
        filter_exec_data->filter_command = (char*)std::calloc(1, (std::strlen(filterCommand)+1)*sizeof(char));
        std::strcpy(filter_exec_data->filter_command, filterCommand);
      }
      
      if (filterPreviewCommand) {
        if (filter_exec_data->filter_preview_command) std::free(filter_exec_data->filter_preview_command);
        filter_exec_data->filter_preview_command = (char*)std::calloc(1, (std::strlen(filterPreviewCommand)+1)*sizeof(char));
        std::strcpy(filter_exec_data->filter_preview_command, filterPreviewCommand);
      }
      
      if (outputParams) {
        if (filter_exec_data->filter_params) std::free(filter_exec_data->filter_params);
//        filter_exec_data->filter_params = buildOutputParamsFromList(*outputParams);
        filter_exec_data->filter_params = GmicLibParser::qstring_2_char(GmicLibParser::buildArgumentsFromParamsList(*outputParams));
        
        if (filter_exec_data->filter_exec_data) std::free(filter_exec_data->filter_exec_data);
        filter_exec_data->filter_exec_data = GmicLibParser::buildFilterData(filterName, filterCommand, filterPreviewCommand, *outputParams, filter_exec_data->filter_data_size);
      }*/
      GmicLibParser::updateFilterExecData(filter_exec_data, filterName, filterCommand, filterPreviewCommand, paramsValues);
    }
  }
}

