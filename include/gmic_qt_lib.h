/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file gmic_qt_lib.h
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
#ifndef _GMIC_QT_GMIC_QT_LIB_H_
#define _GMIC_QT_GMIC_QT_LIB_H_


typedef enum gmic_filter_param_type_t {
	gmic_param_none,
	gmic_param_int,
	gmic_param_float,
	gmic_param_bool,
	gmic_param_choise,
	gmic_param_color,
	gmic_param_separator,
	gmic_param_note,
	gmic_param_file,
	gmic_param_folder,
	gmic_param_text,
	gmic_param_link,
	gmic_param_value,
	gmic_param_button
} gmic_filter_param_type_t;

typedef struct gmic_qt_lib_t
{
  char *std_lib;
  int std_lib_size;
} gmic_qt_lib_t;

typedef struct gmic_filter_execution_data_t
{
  float *image;
  int width;
  int height;
  int spectrum;
  int return_width;
  int return_height;
  int return_spectrum;
  char *filter_name;
  char *filter_command;
  char *filter_preview_command;
  char *filter_params;
  char *filter_data;
  int filter_data_size;
  int return_value;
  int return_window_value;
} gmic_filter_execution_data_t;

typedef struct gmic_filter_param_t
{
	char *param_name;
	int param_type;
	int is_actual_parameter;
	float n_default_value, n_min_value, n_max_value;
	int list_size;
	char *str_default_value;
	char *str_url;
	float rgb_default_value[4];
	int rgb_use_alpha;
	
	char *str_current_value;
	float n_current_value;
	float rgb_current_value[4];
} gmic_filter_param_t;

typedef struct gmic_filter_definition_t
{
  char *filter_name;
  char *filter_command;
  char *filter_preview_command;
  int num_params;
  gmic_filter_param_t * params;
} gmic_filter_definition_t;


gmic_qt_lib_t * gmic_qt_init();

void gmic_qt_free(gmic_qt_lib_t * gmic_qt);


gmic_filter_execution_data_t * gmic_launchPluginCommand(const char * filter_data, int filter_data_size, 
																							float *image, int image_width, int image_height, int image_spectrum, float imageScale);

gmic_filter_execution_data_t * gmic_launchPluginHeadless(gmic_qt_lib_t * gmic_qt, const char * filter_data, int filter_data_size, 
																							float *image, int image_width, int image_height, int image_spectrum, float imageScale);

void gmic_freeFilterExecData(gmic_filter_execution_data_t * filter_exec_data);


gmic_filter_definition_t * gmic_getFilterDefinitionFromFilterData(gmic_qt_lib_t * gmic_qt, const char * filter_data, int filter_data_size);

gmic_filter_execution_data_t * gmic_getFilterExecDataFromFilteDefinition(gmic_filter_definition_t * filter_definition);

void gmic_freeDefinition(gmic_filter_definition_t * filter_definition);


#endif // _GMIC_QT_GMIC_QT_LIB_H_
