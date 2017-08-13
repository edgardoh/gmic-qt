/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file GmicLibParser.cpp
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

#include <cstring>
#include <QList>
#include <QColor>
#include <QBuffer>
#include <QLocale>
#include "host.h"
#include "GmicStdlibParser.h"
#include "GmicLibParser.h"
using namespace cimg_library;


char * GmicLibParser::qstring_2_char(const QString& string)
{
	char * str_char = (char*)std::calloc(1, (string.size()+1)*sizeof(char));
	if (str_char) std::strcpy(str_char, string.toLatin1().constData());
	return str_char;
}

// given a text that points to a G'MIC parameter definition and
// type beign the string of the parameter's type
// returns a list with parameter name and values
QList<QString> GmicLibParser::parseText(const QString & type, const char * text, int & length)
{
  QList<QString> result;
  QString str = text;
  result << str.left(str.indexOf("=")).trimmed();

  QRegExp re( QString("^[^=]*\\s*=\\s*(_?)%1\\s*(.)").arg(type) );
  re.indexIn(str);
  int prefixLength = re.matchedLength();

  QString open = re.cap(2);
  const char * end = 0;
  if ( open == "(" ) {
    end = strstr(text + prefixLength,")");
  } else if ( open == "{") {
    end = strstr(text + prefixLength,"}");
  } else if ( open == "[") {
    end = strstr(text + prefixLength,"]");
  }
  QString values = str.mid(prefixLength,-1).left(end-(text+prefixLength)).trimmed();
  length = 1 + end - text;
  while ( text[length] && (text[length] == ',' || str[length].isSpace() ) ) {
    ++length;
  }
  result << values;
  return result;
}

// sets the default values in filter_params given a text that points to a G'MIC parameter definition
void GmicLibParser::initParameterFromText(const QString & paramType, const char * text, int & textLength, gmic_filter_param_t* filter_params)
{
  textLength = 0;
  QList<QString> list = parseText(paramType, text, textLength);

  filter_params->is_actual_parameter = true;
  
  filter_params->param_name = qstring_2_char(list[0]);
	
  if (paramType == "int" || paramType == "float") {
  	QList<QString> values = list[1].split(QChar(','));
//  	filter_params->str_default_value = qstring_2_char(values[0]);
  	filter_params->n_default_value = filter_params->n_current_value = values[0].toFloat();
  	filter_params->n_min_value = values[1].toFloat();
  	filter_params->n_max_value = values[2].toFloat();
  } else if (paramType == "bool") {
  	filter_params->n_default_value = filter_params->n_current_value = ( list[1].startsWith("true") || list[1].startsWith("1") ) ? 1: 0;
  } else if (paramType == "choice") {
  	QList<QString> choices = list[1].split(QChar(','));
    bool ok;
    int choise = choices[0].toInt(&ok);
    if ( ! ok ) {
    	choise = 0;
    } else {
      choices.pop_front();
    }
    filter_params->n_default_value = filter_params->n_current_value = choise;
    
		QList<QString>::iterator it = choices.begin();
		while ( it != choices.end() ) {
			*it = it->trimmed().remove(QRegExp("^\"")).remove(QRegExp("\"$"));
			++it;
		}
		
		QList<QString>& choises_list = choices;
		filter_params->list_size = choises_list.size();
		int list_len = 0;
		for (int j = 0; j < filter_params->list_size; j++)
			list_len += choises_list[j].size() + 1;
		
		filter_params->str_default_value = (char*)std::calloc(1, list_len*sizeof(char));
		
		char *str_ptr = filter_params->str_default_value;
		for (int j = 0; j < filter_params->list_size; j++) {
			std::strcpy(str_ptr, choises_list[j].toLatin1().constData());
			str_ptr += choises_list[j].size() + 1;
		}

  } else if (paramType == "color") {
  	bool alphaChannel = false;
  	QColor value;
    QList<QString> channels = list[1].split(",");
    int r = channels[0].toInt();
    int g = channels[1].toInt();
    int b = channels[2].toInt();
    if (channels.size() == 4) {
      int a = channels[3].toInt();
      value = QColor(r,g,b,a);
      alphaChannel = true;
    } else {
     value = QColor(r,g,b);
    }
    filter_params->rgb_default_value[0] = filter_params->rgb_current_value[0] = value.red();
    filter_params->rgb_default_value[1] = filter_params->rgb_current_value[1] = value.green();
    filter_params->rgb_default_value[2] = filter_params->rgb_current_value[2] = value.blue();
    filter_params->rgb_default_value[3] = filter_params->rgb_current_value[3] = (alphaChannel) ? value.alpha(): 1.f;
    filter_params->rgb_use_alpha = alphaChannel;
    
/*    const QColor & c = value;
    QString rgb_color;
    if (alphaChannel)
    	rgb_color = QString("%1,%2,%3,%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
    else
    	rgb_color = QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
    filter_params->str_default_value = qstring_2_char( rgb_color );
    */
  } else if (paramType == "file" || paramType == "folder" || paramType == "value") {
  	filter_params->str_default_value = qstring_2_char( list[1] );
  } else if (paramType == "text") {
  	QString value = list[1];
  	// TODO: should we return multiline?
//    bool _multiline = false;
    QRegExp re("^\\s*(0|1)\\s*,");
    if ( value.contains(re) && re.matchedLength() > 0 ) {
//      _multiline = (re.cap(1).toInt() == 1);
      value.replace(re,"");
    }
    value = value.trimmed().remove(QRegExp("^\"")).remove(QRegExp("\"$"));
    value.replace(QString("\\\\"),QString("\\"));
    value.replace(QString("\\n"),QString("\n"));
    filter_params->str_default_value = qstring_2_char( value );
  } else if (paramType == "button") {
  	// FIXME: not sure how this work...
//  	str_default = QString("0");
  	filter_params->n_default_value = filter_params->n_current_value = 0;
  } else if (paramType == "separator") {
  	filter_params->is_actual_parameter = false;
  } else if (paramType == "note") {
  	filter_params->is_actual_parameter = false;
  	
    QString str_text = list[1].trimmed()
        .remove(QRegExp("^\""))
        .remove(QRegExp("\"$"))
        .replace(QString("\\\""),"\"");
    str_text.replace(QString("\\n"),"<br/>");
    str_text.replace(QRegExp("color\\s*=\\s*\""),QString("style=\"color:"));
    str_text.replace(QRegExp("foreground\\s*=\\s*\""),QString("style=\"color:"));

    filter_params->str_default_value = qstring_2_char( str_text );
  } else if (paramType == "link") {
  	filter_params->is_actual_parameter = false;
  	
  	QString str_text, str_url;
    QList<QString> values = list[1].split(QChar(','));

    // TODO: should we return alignment
/*    if ( values.size() == 3 ) {
      float a = values[0].toFloat();
      if ( a == 0.0f ) {
        _alignment = Qt::AlignLeft;
      } else if ( a == 1.0f ) {
        _alignment = Qt::AlignRight;
      } else {
        _alignment = Qt::AlignCenter;
      }
      values.pop_front();
    } else {
      _alignment = Qt::AlignCenter;
    }
*/
    if ( values.size() == 2 ) {
      str_text = values[0].trimmed().remove(QRegExp("^\"")).remove(QRegExp("\"$"));
      values.pop_front();
    }
    if ( values.size() == 1 ) {
      str_url = values[0].trimmed().remove(QRegExp("^\"")).remove(QRegExp("\"$"));
    }
    if ( str_text.isEmpty() ) {
      str_text = str_url;
    }
    filter_params->str_default_value = qstring_2_char( str_text );
    filter_params->str_url = qstring_2_char( str_url );
  }
  else {
  	std::free(filter_params->param_name);
  	filter_params->param_name = 0;
  	filter_params->is_actual_parameter = false;
  }
}

// given a text with a G'MIC definition of a parameters it parses it and return the values in a series of lists
void GmicLibParser::parseParameterFromText(const char * text, int& length, QString& error, gmic_filter_param_t* filter_params)
{
  QString line = text;
  
  error.clear();
  filter_params->param_type = gmic_param_none;
  filter_params->is_actual_parameter = 0;
  QString paramTypeStr;
  
#define PREFIX "^[^=]*\\s*=\\s*_?"
  if ( QRegExp( PREFIX "int").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_int;
    paramTypeStr = "int";
  } else if ( QRegExp( PREFIX "float").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_float;
    paramTypeStr = "float";
  } else if ( QRegExp( PREFIX "bool").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_bool;
    paramTypeStr = "bool";
  } else if ( QRegExp( PREFIX "choice").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_choise;
    paramTypeStr = "choice";
  } else if ( QRegExp( PREFIX "color").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_color;
    paramTypeStr = "color";
  } else if ( QRegExp( PREFIX "separator").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_separator;
    paramTypeStr = "separator";
  } else if ( QRegExp( PREFIX "note").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_note;
    paramTypeStr = "note";
  } else if ( QRegExp( PREFIX "file").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_file;
    paramTypeStr = "file";
  } else if ( QRegExp( PREFIX "folder").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_folder;
    paramTypeStr = "folder";
  } else if ( QRegExp( PREFIX "text").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_text;
    paramTypeStr = "text";
  } else if ( QRegExp( PREFIX "link").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_link;
    paramTypeStr = "link";
  } else if ( QRegExp( PREFIX "value").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_value;
    paramTypeStr = "value";
  } else if ( QRegExp( PREFIX "button").indexIn(line) == 0 ) {
    filter_params->param_type = gmic_param_button;
    paramTypeStr = "button";
  }
  if (filter_params->param_type != gmic_param_none) {
  	initParameterFromText(paramTypeStr, text, length, filter_params);
  	
		if ( filter_params->param_name == 0 && !line.isEmpty() ) {
			error = QString("parseParameterFromText(): Parse error: %1").arg(line);
		}
  }

}

// parses a G'MIC definition of the parameters, usually returned by getFilterDefinition()
// and fills gmic_filter_param_t* in filter_definition
int GmicLibParser::parseFilterDefinitionParams(gmic_filter_definition_t * filter_definition, const QString& defaultParams)
{
	int resp = 0;
	
  QByteArray rawText = defaultParams.toLatin1();
  const char * cstr = rawText.constData();
  int length;
//  int parametersCount = 0;
  QList<gmic_filter_param_t> filter_params_list;
	gmic_filter_param_t filter_params;

  // Build parameters
  QString error;
  do {
  	std::memset(&filter_params, 0, sizeof(filter_params));

    parseParameterFromText(cstr, length, error, &filter_params);
    if ( filter_params.param_type != gmic_param_none ) {
//        parametersCount++;
        filter_params_list.push_back(filter_params);
    }
    cstr += length;
  } while (filter_params.param_type != gmic_param_none && error.isEmpty());

	filter_definition->num_params = filter_params_list.size();
	if (filter_definition->num_params > 0) {
		filter_definition->params = (gmic_filter_param_t*)std::calloc(filter_definition->num_params, sizeof(gmic_filter_param_t));
		
		for (int i = 0; i < filter_definition->num_params; i++) {
			filter_definition->params[i] = filter_params_list[i];
		}
	}

  if ( !error.isEmpty() ) {
//  	parametersCount = 0;
  	resp = 1;
  }

  return resp;
}

void GmicLibParser::buildParamsListFromOutput(QList<QString>& outputParamsList, const char * outputParams, int params_size)
{
	outputParamsList.clear();
	
//	int params_count = 0;
	
	int i = 0;
	const char *params = outputParams;
	while ( i < params_size ) {
		outputParamsList.push_back( params );
		int inc = std::strlen(params) + 1;
		i += inc;
		if ( i < params_size ) params += inc;
		
//		params_count++;
	}
/*	if ( i < params_size ) {
		outputParamsList.push_back( params );
		
//		params_count++;
	}
	*/
}

char * GmicLibParser::buildFilterData(const char * filterName, const char * filterCommand, const char * filterPreviewCommand, 
		const QList<QString>& paramsList, int& filter_data_size)
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

// given a list of parameters values returns a string that can be used to execute a G'MIC command
QString GmicLibParser::buildArgumentsFromParamsList(const QList<QString>& outputParamsList)
{
	return outputParamsList.join(",");
}

void GmicLibParser::parseFilterData(const char * filterData, int filterDataSize, QString& name, QString& command, QString& previewCommand, QList<QString>& paramValues)
{
	name.clear();
	command.clear();
	previewCommand.clear();
	paramValues.clear();
	
	int inc;
	int current_size = 0;
	const char *data = filterData;
	
	if (data) {
		name = QString(data);
		inc = name.size()+1;
		current_size += inc;
		if ( current_size < filterDataSize )
			data += inc;
		else
			data = 0;
	}
	if (data) {
		command = QString(data);
		inc = command.size()+1;
		current_size += inc;
		if ( current_size < filterDataSize )
			data += inc;
		else
			data = 0;
	}
	
	if (data) {
		previewCommand = QString(data);
		inc = previewCommand.size()+1;
		current_size += inc;
		if ( current_size < filterDataSize )
			data += inc;
		else
			data = 0;
	}
	
	if (data) {
		while ( current_size < filterDataSize ) {
			paramValues.push_back( data );
			int inc = std::strlen(data) + 1;
			current_size += inc;
			if ( current_size < filterDataSize ) data += inc;
		}
	}
}

gmic_filter_execution_data_t * GmicLibParser::getFilterExecDataFromFilteDefinition(gmic_filter_definition_t * filter_definition)
{
	if (filter_definition == 0) return 0;
	
	gmic_filter_execution_data_t * filter_exec_data = (gmic_filter_execution_data_t*)std::calloc(1, sizeof(gmic_filter_execution_data_t));
	int parametersCount = filter_definition->num_params;
	gmic_filter_param_t * filter_params = filter_definition->params;
	
	QList<QString> paramValues;
	QString name, command, previewCommand;

	if (filter_definition->filter_name) name = QString(filter_definition->filter_name);
	if (filter_definition->filter_command) command = QString(filter_definition->filter_command);
	if (filter_definition->filter_preview_command) previewCommand = QString(filter_definition->filter_preview_command);
	
	for (int i = 0; i < parametersCount; i++) {
		if (filter_params[i].is_actual_parameter) {
				if ( filter_params[i].param_type == gmic_param_int ||
						filter_params[i].param_type == gmic_param_float ||
						filter_params[i].param_type == gmic_param_bool ||
						filter_params[i].param_type == gmic_param_choise ) {
					paramValues.push_back( QString("%1").arg(filter_params[i].n_current_value) );

				} else if ( filter_params[i].param_type == gmic_param_color ) {
					if (filter_params[i].rgb_use_alpha)
						paramValues.push_back( QString("%1,%2,%3,%4").arg(filter_params[i].rgb_current_value[0]).
								arg(filter_params[i].rgb_current_value[1]).
								arg(filter_params[i].rgb_current_value[2]).
								arg(filter_params[i].rgb_current_value[3]));
					else
						paramValues.push_back( QString("%1,%2,%3").arg(filter_params[i].rgb_current_value[0]).
								arg(filter_params[i].rgb_current_value[1]).
								arg(filter_params[i].rgb_current_value[2]) );
				} else if ( filter_params[i].param_type == gmic_param_separator ) {

				} else if ( filter_params[i].param_type == gmic_param_note || 
						filter_params[i].param_type == gmic_param_file || 
						filter_params[i].param_type == gmic_param_folder || 
						filter_params[i].param_type == gmic_param_text || 
						filter_params[i].param_type == gmic_param_link ||
						filter_params[i].param_type == gmic_param_value ) {
					if (filter_params[i].str_current_value)
						paramValues.push_back( filter_params[i].str_current_value );
					else if (filter_params[i].str_default_value)
						paramValues.push_back( filter_params[i].str_default_value );
					else
						paramValues.push_back( "" );
				} else if ( filter_params[i].param_type == gmic_param_button ) {
					paramValues.push_back( QString("%1").arg(filter_params[i].n_current_value) );
				}
		}
	}
	
	filter_exec_data->filter_data = GmicLibParser::buildFilterData(filter_definition->filter_name, filter_definition->filter_command, filter_definition->filter_preview_command, 
			paramValues, filter_exec_data->filter_data_size);
	
	return filter_exec_data;
}

void GmicLibParser::updateFilterExecData(gmic_filter_execution_data_t * filter_exec_data, const QString& filterName, 
		const QString& filterCommand, const QString& filterPreviewCommand, const QList<QString>& paramsValues)
{
	if (filter_exec_data == 0) return;
	
	if (filter_exec_data->filter_name) std::free(filter_exec_data->filter_name);
	filter_exec_data->filter_name = (char*)std::calloc(1, (filterName.size()+1)*sizeof(char));
	std::strcpy(filter_exec_data->filter_name, filterName.toLatin1().constData());

	if (filter_exec_data->filter_command) std::free(filter_exec_data->filter_command);
	filter_exec_data->filter_command = (char*)std::calloc(1, (filterCommand.size()+1)*sizeof(char));
	std::strcpy(filter_exec_data->filter_command, filterCommand.toLatin1().constData());

	if (filter_exec_data->filter_preview_command) std::free(filter_exec_data->filter_preview_command);
	filter_exec_data->filter_preview_command = (char*)std::calloc(1, (filterPreviewCommand.size()+1)*sizeof(char));
	std::strcpy(filter_exec_data->filter_preview_command, filterPreviewCommand.toLatin1().constData());

	if (filter_exec_data->filter_params) std::free(filter_exec_data->filter_params);
	filter_exec_data->filter_params = GmicLibParser::qstring_2_char(GmicLibParser::buildArgumentsFromParamsList(paramsValues));
	
	if (filter_exec_data->filter_data) std::free(filter_exec_data->filter_data);
	filter_exec_data->filter_data = GmicLibParser::buildFilterData(filterName.toLatin1().constData(), filterCommand.toLatin1().constData(), 
																														filterPreviewCommand.toLatin1().constData(), paramsValues, filter_exec_data->filter_data_size);
	
}

void GmicLibParser::freeFilterExecData(gmic_filter_execution_data_t * filter_exec_data)
{
  if (filter_exec_data) {
    if (filter_exec_data->filter_name) std::free(filter_exec_data->filter_name);
    if (filter_exec_data->filter_command) std::free(filter_exec_data->filter_command);
    if (filter_exec_data->filter_preview_command) std::free(filter_exec_data->filter_preview_command);
    if (filter_exec_data->filter_params) std::free(filter_exec_data->filter_params);
    if (filter_exec_data->filter_data) std::free(filter_exec_data->filter_data);

    std::free(filter_exec_data);
  }
}

gmic_filter_definition_t * GmicLibParser::getFilterDefinition(const QString& ifilterName, const QString& ifilterCommand, const QString& ifilterPreviewCommand)
{
//	int resp = 0;
	gmic_filter_definition_t * filter_definition = (gmic_filter_definition_t*)std::calloc(1, sizeof(gmic_filter_definition_t));
	if (filter_definition == 0) return 0;
	
	QString ifilterDefinition;
	
	// TODO: should we return preview factor?
//	float ipreviewFactor = GmicQt::PreviewFactorAny;
//	int iaccurateIfZoomed = 0;
	
  QBuffer stdlib(&GmicStdLibParser::GmicStdlib);
  stdlib.open(QBuffer::ReadOnly);
//  QList<QStandardItem*> treeFoldersStack;
  QList<QString> filterPath;
  bool bFound = false;

  QString language;

  QList<QString> languages = QLocale().uiLanguages();
  if ( languages.size() ) {
    language = languages.front().split("-").front();
  } else {
    language = "void";
  }
  if ( ! GmicStdLibParser::GmicStdlib.contains(QString("#@gui_%1").arg(language).toLocal8Bit()) ) {
    language = "en";
  }

  // Use _en locale if not localization for the language is found.

  QString buffer = stdlib.readLine(4096);
  QString line;

  QRegExp folderRegexpNoLanguage("^..gui[ ][^:]+$");
  QRegExp folderRegexpLanguage( QString("^..gui_%1[ ][^:]+$").arg(language));

  QRegExp filterRegexpNoLanguage("^..gui[ ][^:]+[ ]*:.*");
  QRegExp filterRegexpLanguage( QString("^..gui_%1[ ][^:]+[ ]*:.*").arg(language));

  const QChar WarningPrefix('!');
  do {
    line = buffer.trimmed();
    if ( line.startsWith("#@gui") ) {
      if ( filterRegexpNoLanguage.exactMatch(line) || filterRegexpLanguage.exactMatch(line) ) {
        //
        // A filter
        //
        QString filterName = line;
        filterName.replace( QRegExp("[ ]*:.*$"), "" );
        filterName.replace( QRegExp("^..gui[_a-zA-Z]{0,3}[ ]"), "" );

        const bool warning = filterName.startsWith(WarningPrefix);
        if ( warning ) {
          filterName.remove(0,1);
        }

        QString filterCommands = line;
        filterCommands.replace(QRegExp("^..gui[_a-zA-Z]{0,3}[ ][^:]+[ ]*:[ ]*"),"");

        QList<QString> commands = filterCommands.split(",");

        QString filterCommand = commands[0].trimmed();
        
					QList<QString> preview = commands[1].trimmed().split("(");
					
					// TODO: should we return preview factor?
/*					float previewFactor = GmicQt::PreviewFactorAny;
					bool accurateIfZoomed = true;
					if ( preview.size() >= 2 ) {
						if (preview[1].endsWith("+")) {
							accurateIfZoomed = true;
							preview[1].chop(1);
						} else {
							accurateIfZoomed = false;
						}
						previewFactor = preview[1].replace(QRegExp("\\).*"),"").toFloat();
					}
					
					ipreviewFactor = previewFactor;
					iaccurateIfZoomed = accurateIfZoomed;
					*/
					QString filterPreviewCommand = preview[0].trimmed();
					
	        if ( ifilterName == filterName && filterCommand == ifilterCommand && ifilterPreviewCommand == filterPreviewCommand ) {
	        	
					QString start = line;
					start.replace(QRegExp(" .*")," :");
	
					// Read parameters
					QString parameters;
					do {
						buffer = stdlib.readLine(4096);
						if ( buffer.startsWith(start) ) {
							QString parameterLine = buffer;
							parameterLine.replace(QRegExp("^..gui[_a-zA-Z]{0,3}[ ]*:[ ]*"), "");
							parameters += parameterLine;
						}
					} while ( (buffer.startsWith(start)
										 || buffer.startsWith("#")
										 || (buffer.trimmed().isEmpty() && ! stdlib.atEnd()))
										&& !folderRegexpNoLanguage.exactMatch(buffer)
										&& !folderRegexpLanguage.exactMatch(buffer)
										&& !filterRegexpNoLanguage.exactMatch(buffer)
										&& !filterRegexpLanguage.exactMatch(buffer));
					
					ifilterDefinition = parameters;
				
					bFound = true;
				}
      }
    }
    
    if (!bFound) buffer = stdlib.readLine(4096);
  } while ( ! buffer.isEmpty() && !bFound );

  if (bFound) {
		if (!parseFilterDefinitionParams(filter_definition, ifilterDefinition)) {
			filter_definition->filter_name = qstring_2_char(ifilterName);
			
			filter_definition->filter_command = qstring_2_char(ifilterCommand);
			
			filter_definition->filter_preview_command = qstring_2_char(ifilterPreviewCommand);
		} else {
			bFound = false;
	  }
  }
  if (!bFound && filter_definition != nullptr) {
  	GmicLibParser::freeDefinition(filter_definition);
  	filter_definition = 0;
  }
  
  return filter_definition;
}

int GmicLibParser::updateFilteDefinitionFromFilterData(gmic_filter_definition_t * filter_definition, const char * filter_data, int filter_data_size)
{
	int arguments_ok = 1;
	int parametersCount = filter_definition->num_params;
	gmic_filter_param_t * filter_params = filter_definition->params;
	
	QList<QString> arguments;
	QString name, command, previewCommand;
	
	parseFilterData(filter_data, filter_data_size, name, command, previewCommand, arguments);

	int outIndex = 0;
	
	for (int i = 0; i < parametersCount; i++) {
		if (filter_params[i].is_actual_parameter) {
			if (outIndex < arguments.size()) {
				if ( filter_params[i].param_type == gmic_param_int ||
						filter_params[i].param_type == gmic_param_float ||
						filter_params[i].param_type == gmic_param_bool ||
						filter_params[i].param_type == gmic_param_choise ) {
					filter_params[i].n_current_value = arguments[outIndex++].toFloat();

				} else if ( filter_params[i].param_type == gmic_param_color ) {
					QList<QString> colors = arguments[outIndex++].split(",");
					int toColor = (filter_params[i].rgb_use_alpha) ? 4: 3;
					if (toColor == colors.size()) {
						filter_params[i].rgb_current_value[3] = 1.f;
						for (int iColor = 0; iColor < toColor; iColor++) {
							filter_params[i].rgb_current_value[iColor] = colors[iColor].toFloat();
						}
					} else {
						arguments_ok = 0;
						break;
					}
				} else if ( filter_params[i].param_type == gmic_param_separator ) {

				} else if ( filter_params[i].param_type == gmic_param_note || 
						filter_params[i].param_type == gmic_param_file || 
						filter_params[i].param_type == gmic_param_folder || 
						filter_params[i].param_type == gmic_param_text || 
						filter_params[i].param_type == gmic_param_link ||
						filter_params[i].param_type == gmic_param_value ) {
					if (filter_params[i].str_current_value) std::free( filter_params[i].str_current_value );
					filter_params[i].str_current_value = qstring_2_char(arguments[outIndex++]);

				} else if ( filter_params[i].param_type == gmic_param_button ) {
					filter_params[i].n_current_value = arguments[outIndex++].toFloat();
				}
			} else {
				arguments_ok = 0;
				break;
			}
		}
	}
	
	return arguments_ok;
}

void GmicLibParser::freeDefinition(gmic_filter_definition_t * filter_definition)
{
	if (filter_definition) {
		if (filter_definition->filter_name) std::free(filter_definition->filter_name);
		if (filter_definition->filter_command) std::free(filter_definition->filter_command);
		if (filter_definition->filter_preview_command) std::free(filter_definition->filter_preview_command);
		
		if (filter_definition->params) {
	  	for (int i = 0; i < filter_definition->num_params; i++) {
	  		gmic_filter_param_t *param = filter_definition->params + i;
	  		
	  		if (param->param_name) std::free(param->param_name);
	  		if (param->str_current_value) std::free(param->str_current_value);
	  		if (param->str_default_value) std::free(param->str_default_value);
	  		if (param->str_url) std::free(param->str_url);
	  	}
	  	
	  	std::free(filter_definition->params);
		}
		
		std::free(filter_definition);
	}

}

void 
GmicLibParser::runFilter(gmic_filter_execution_data_t * filter_exec_data, 
									const QString& filterName,
									const QString& filterCommand,
									const QString& filterPreviewCommand,
									const QList<QString>& paramsValues, 
									bool& failed, 
									GmicQt::OutputMessageMode outputMessageMode,
									GmicQt::InputMode inputMode,
									GmicQt::OutputMode outputMode,
									const float imageScale)
{
  cimg_library::CImgList<float> images;
  cimg_library::CImgList<char> imageNames;
  QString environment;
  GmicQt::OutputMessageMode messageMode = outputMessageMode;
  bool gmicAbort;
  float gmicProgress;
  QString gmicStatus;
  QString errorMessage;
  
  gmic_qt_get_cropped_images(images,imageNames,-1,-1,-1,-1,inputMode,filter_exec_data);
  
  errorMessage.clear();
  failed = false;
  if (!images) {
    images.assign(1);
    imageNames.assign(1);
  }
  QString fullCommandLine;
  try {
    if ( messageMode == GmicQt::Quiet ) {
      fullCommandLine = QString("-v -");
    } else if ( messageMode >= GmicQt::VerboseLayerName && messageMode <= GmicQt::VerboseLogFile ) {
      fullCommandLine = QString("-v -99");
    } else if ( messageMode == GmicQt::VeryVerboseConsole || messageMode == GmicQt::VeryVerboseLogFile  ) {
      fullCommandLine = QString("-v 0");
    } else if ( messageMode == GmicQt::DebugConsole || messageMode == GmicQt::DebugLogFile  ) {
      fullCommandLine = QString("-debug") ;
    }
    fullCommandLine += QString(" _IMAGE_SCALE=%1").arg(imageScale);
    fullCommandLine += QString(" -%1 %2").arg(filterCommand).arg(GmicLibParser::buildArgumentsFromParamsList(paramsValues));
    gmicAbort = false;
    gmicProgress = -1;
    if (messageMode > GmicQt::Quiet) {
      std::fprintf(cimg::output(),"\n[gmic_qt] Command: %s\n",
                   fullCommandLine.toLatin1().constData());
      std::fflush(cimg::output());
    }

    gmic gmicInstance(environment.isEmpty() ? 0 : QString("-v - %1").arg(environment).toLatin1().constData(),
                      GmicStdLibParser::GmicStdlib.constData(),true);
    gmicInstance.set_variable("_host",GmicQt::HostApplicationShortname,'=');
    gmicInstance.run(fullCommandLine.toLatin1().constData(),
                     images,
                     imageNames,
                     &gmicProgress,
                     &gmicAbort);
    gmicStatus = gmicInstance.status;
    
    gmic_qt_output_images(images,
                          imageNames,
                          outputMode,
													filterName,
													filterCommand,
													filterPreviewCommand,
													paramsValues,
													filter_exec_data,
                          (outputMessageMode == GmicQt::VerboseLayerName) ?
                            QString("[G'MIC] %1: %2")
                            .arg(filterName)
                            .arg(fullCommandLine)
                            .toLatin1().constData(): 0);
  } catch (gmic_exception & e) {
    images.assign();
    imageNames.assign();
    const char * message = e.what();
    errorMessage = message;
    if (messageMode > GmicQt::Quiet) {
      std::fprintf(cimg::output(),"\n[gmic_qt]./error/ When running command '%s', this error occured:\n%s\n",
                   fullCommandLine.toLatin1().constData(),
                   message);
      std::fflush(cimg::output());
    }
    failed = true;
  }
}

