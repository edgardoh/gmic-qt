/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file GmicLibParser.h
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
#ifndef _GMIC_QT_GMICLIBPARSER_H_
#define _GMIC_QT_GMICLIBPARSER_H_

#include <QString>
#include "gmic_qt.h"
#include "gmic.h"
#include "gmic_qt_lib.h"


class GmicLibParser
{
public:
	GmicLibParser();
	
	static void parseFilterData(const char * filterData, int filterDataSize, QString& name, QString& command, QString& previewCommand, QList<QString>& paramValues);
	
	static gmic_filter_execution_data_t * getFilterExecDataFromFilteDefinition(gmic_filter_definition_t * filter_definition);

	static void updateFilterExecData(gmic_filter_execution_data_t * filter_exec_data, const QString& filterName, 
			const QString& filterCommand, const QString& filterPreviewCommand, const QList<QString>& paramsValues);
	
	static void freeFilterExecData(gmic_filter_execution_data_t * filter_exec_data);

	static gmic_filter_definition_t * getFilterDefinition(const QString& ifilterName, const QString& ifilterCommand, const QString& ifilterPreviewCommand);

	static int updateFilteDefinitionFromFilterData(gmic_filter_definition_t * filter_definition, const char * filter_data, int filter_data_size);

	static void freeDefinition(gmic_filter_definition_t * filter_definition);

  static void runFilter(gmic_filter_execution_data_t * filter_exec_data, 
						const QString& filterName,      
						const QString& filterCommand,
						const QString& filterPreviewCommand,
						const QList<QString>& paramsValues, 
            bool& failed, 
            GmicQt::OutputMessageMode outputMessageMode,
            GmicQt::InputMode inputMode,
            GmicQt::OutputMode outputMode,
						const float imageScale);

private:
	static char * qstring_2_char(const QString& string);
	static QList<QString> parseText(const QString & type, const char * text, int & length);
	static void initParameterFromText(const QString & paramType, const char * text, int & textLength, gmic_filter_param_t* filter_params);
	static void parseParameterFromText(const char * text, int& length, QString& error, gmic_filter_param_t* filter_params);
	static int parseFilterDefinitionParams(gmic_filter_definition_t * filter_definition, const QString& defaultParams);
	static void buildParamsListFromOutput(QList<QString>& outputParamsList, const char * outputParams, int params_size);
	static char * buildFilterData(const char * filterName, const char * filterCommand, const char * filterPreviewCommand, 
			const QList<QString>& paramsList, int& filter_data_size);
	static QString buildArgumentsFromParamsList(const QList<QString>& outputParamsList);
  
};

#endif // _GMIC_QT_GMICLIBPARSER_H_
