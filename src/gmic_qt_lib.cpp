/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file gmic_qt_lib.cpp
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

#include <QApplication>
#include <QStyleFactory>
#include <QTranslator>
#include "Updater.h"
#include "MainWindow.h"
#include "gmic_qt_lib.h"
#include "GmicStdlibParser.h"
#include "GmicLibParser.h"

// begin gmic_qt_library

void unquoteStringValues(QList<QString>& paramValues)
{
	for (int i = 0; i < paramValues.size(); i++) {
		QString& str = paramValues[i];
		if (str.startsWith('\"') && str.endsWith('\"')) {
			paramValues[i] = str.mid(1, str.size()-2);
		}
	}
}

gmic_qt_lib_t * gmic_qt_init()
{
	gmic_qt_lib_t * gmic_qt = NULL;
	
	QObject app;
	Updater::setInstanceParent(&app);
	Updater::getInstance()->updateSources(false);
	GmicStdLibParser::GmicStdlib = Updater::getInstance()->buildFullStdlib();
	if (GmicStdLibParser::GmicStdlib.size() > 0) {
		gmic_qt = (gmic_qt_lib_t *)std::malloc(sizeof(gmic_qt_lib_t));
		
		gmic_qt->std_lib_size = GmicStdLibParser::GmicStdlib.size();
		gmic_qt->std_lib = (char*)std::malloc(gmic_qt->std_lib_size);
		if (gmic_qt->std_lib) {
			std::memcpy(gmic_qt->std_lib, GmicStdLibParser::GmicStdlib.constData(), gmic_qt->std_lib_size);
		}
	}
	
	return gmic_qt;
}

void gmic_qt_free(gmic_qt_lib_t * gmic_qt)
{
	if (gmic_qt) {
		if (gmic_qt->std_lib) std::free(gmic_qt->std_lib);
		
		std::free(gmic_qt);
	}
}

gmic_filter_execution_data_t * gmic_launchPluginCommand(const char * filter_data, int filter_data_size, 
																							float *image, int image_width, int image_height, int image_spectrum, float imageScale)
{
  gmic_filter_execution_data_t * filter_exec_data = (gmic_filter_execution_data_t*)std::calloc(1, sizeof(gmic_filter_execution_data_t));
   
  int dummy_argc = 1;
  char dummy_app_name[] = GMIC_QT_APPLICATION_NAME;
  char * dummy_argv[1] = { dummy_app_name };

  filter_exec_data->image = image;
  filter_exec_data->width = image_width;
  filter_exec_data->height = image_height;
  filter_exec_data->spectrum = image_spectrum;

#ifdef _IS_WINDOWS_
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif

  QApplication::setStyle(QStyleFactory::create("Fusion"));
  
  QApplication app(dummy_argc, dummy_argv);
  app.setWindowIcon(QIcon(":resources/gmic_hat.png"));
  QCoreApplication::setOrganizationName(GMIC_QT_ORGANISATION_NAME);
  QCoreApplication::setOrganizationDomain(GMIC_QT_ORGANISATION_DOMAIN);
  QCoreApplication::setApplicationName(GMIC_QT_APPLICATION_NAME);
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

  // Translate according to current locale
  QList<QString> languages = QLocale().uiLanguages();
  if ( languages.size() ) {
    QString lang = languages.front().split("-").front();
    QStringList translations;
    translations << "fr" << "de" << "es" << "zh" << "nl"
                 << "cs" << "it" << "id" << "ua" << "ru"
     << "po" << "pt";
    if ( translations.contains(lang) ) {
      QTranslator * translator = new QTranslator(&app);
      translator->load(QString(":/translations/%1.qm").arg(lang));
      app.installTranslator(translator);
    }
  }

  Updater::setInstanceParent(&app);
  int ageLimit;
  {
    QSettings settings;
    GmicQt::OutputMessageMode mode = static_cast<GmicQt::OutputMessageMode>(settings.value("OutputMessageModeValue",GmicQt::Quiet).toInt());
    Updater::setOutputMessageMode(mode);
    ageLimit = settings.value(INTERNET_UPDATE_PERIODICITY_KEY,0).toInt();
  }
  Updater * updater = Updater::getInstance();
  
  QString *name = 0;
  QString *command = 0;
  QString *previewCommand = 0;
  QList<QString> *paramValues = 0;

  if (filter_data) {
  	name = new QString;
		command = new QString;
		previewCommand = new QString;
		paramValues = new QList<QString>;
		
		GmicLibParser::parseFilterData(filter_data, filter_data_size, *name, *command, *previewCommand, *paramValues);
		unquoteStringValues(*paramValues);
  }
  
  MainWindow mainWindow(filter_exec_data, name, command, previewCommand, paramValues, imageScale);
  
  QObject::connect(updater,SIGNAL(downloadsFinished(bool)),
                   &mainWindow,SLOT(startupUpdateFinished(bool)));
  updater->startUpdate(ageLimit,4, ageLimit != INTERNET_NEVER_UPDATE_PERIODICITY );
  filter_exec_data->return_window_value = app.exec();
  
  if (name) delete name;
  if (command) delete command;
  if (previewCommand) delete previewCommand;
  if (paramValues) delete paramValues;
  
  return filter_exec_data;
}

gmic_filter_execution_data_t * gmic_launchPluginHeadless(gmic_qt_lib_t * gmic_qt, const char * filter_data, int filter_data_size, 
																								float *image, int image_width, int image_height, int image_spectrum, float imageScale)
{
	if (gmic_qt == 0) return 0;
	if (filter_data == 0) return 0;

  gmic_filter_execution_data_t * filter_exec_data = (gmic_filter_execution_data_t*)std::calloc(1, sizeof(gmic_filter_execution_data_t));
  
  filter_exec_data->image = image;
  filter_exec_data->width = image_width;
  filter_exec_data->height = image_height;
  filter_exec_data->spectrum = image_spectrum;

#ifdef _IS_WINDOWS_
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif
  
	GmicStdLibParser::GmicStdlib = QByteArray::fromRawData(gmic_qt->std_lib, gmic_qt->std_lib_size);
	
  bool failed;
  QString name;
  QString command;
  QString previewCommand;
	QList<QString> paramValues;
		
	GmicLibParser::parseFilterData(filter_data, filter_data_size, name, command, previewCommand, paramValues);

	GmicLibParser::runFilter(filter_exec_data, 
							name, 
							command, 
							previewCommand, 
							paramValues, 
							failed, 
							GmicQt::Quiet,
//							GmicQt::VeryVerboseConsole,
							GmicQt::Active,
							GmicQt::InPlace,
							imageScale);

  return filter_exec_data;
}

void gmic_freeFilterExecData(gmic_filter_execution_data_t * filter_exec_data)
{
	GmicLibParser::freeFilterExecData(filter_exec_data);
}

gmic_filter_execution_data_t * gmic_getFilterExecDataFromFilteDefinition(gmic_filter_definition_t * filter_definition)
{
	return GmicLibParser::getFilterExecDataFromFilteDefinition(filter_definition);
}

gmic_filter_definition_t * gmic_getFilterDefinitionFromFilterData(gmic_qt_lib_t * gmic_qt, const char * filter_data, int filter_data_size)
{
	if (gmic_qt == 0) return 0;
	if (filter_data == 0) return 0;
	
	gmic_filter_definition_t * filter_definition = 0;
	
	QString name, command, previewCommand;
	QList<QString> paramValues;
	
	GmicLibParser::parseFilterData(filter_data, filter_data_size, name, command, previewCommand, paramValues);
	
	GmicStdLibParser::GmicStdlib = QByteArray::fromRawData(gmic_qt->std_lib, gmic_qt->std_lib_size);

  filter_definition = GmicLibParser::getFilterDefinition(name, command, previewCommand);
  if (filter_definition) {
  	if (!GmicLibParser::updateFilteDefinitionFromFilterData(filter_definition, filter_data, filter_data_size)) {
  		GmicLibParser::freeDefinition(filter_definition);
    	filter_definition = 0;
  	}
  }
  
  return filter_definition;
}

void gmic_freeDefinition(gmic_filter_definition_t * filter_definition)
{
	GmicLibParser::freeDefinition(filter_definition);
}

// end gmic_qt_library

