/*
Copyright (c) 2007-2008 Michael Specht

This file is part of SimQuant.

SimQuant is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SimQuant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SimQuant.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtCore>
#include <stdio.h>
#include "SpectrumExtractor.h"
#include <ptb/RefPtr.h>
#include "version.h"


void printUsageAndExit()
{
	printf("Usage: extractspectra [spectrum file] [scan id file]\n");
	printf("The spectrum file may be mzML, mzXML or mzData, optionally compressed (.gz, .bz2, .zip).\n");
	printf("The scan id file is a text file that contains one scan id per line.\n");
	exit(1);
}


bool stringToBool(QString& as_String)
{
	if (as_String == "yes" || as_String == "true" || as_String == "on" || as_String == "enable" || as_String == "enabled")
		return true;
	else if (as_String == "no" || as_String == "false" || as_String == "off" || as_String == "disable" || as_String == "disabled")
		return false;
	else
	{
		printf("Error: unknown boolean value '%s'.\n", as_String.toStdString().c_str());
		exit(1);
	}
};


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
	QStringList lk_Arguments;
	for (int i = 1; i < ai_ArgumentCount; ++i)
		lk_Arguments << ac_Arguments__[i];
		
	if (!lk_Arguments.empty() && (lk_Arguments.first() == "--version"))
	{
		printf("extractspectra %s\n", gs_Version.toStdString().c_str());
		exit(0);
	}
		
	if (lk_Arguments.empty())
		printUsageAndExit();
		
	QString ls_SpotPath;
	QString ls_IdListPath;
	
	if (!lk_Arguments.empty())
		ls_SpotPath = lk_Arguments.takeFirst();
	
	if (!lk_Arguments.empty())
		ls_IdListPath = lk_Arguments.takeFirst();
		
	if (ls_SpotPath.isEmpty() || ls_IdListPath.isEmpty())
		printUsageAndExit();
	
	QSet<QString> lk_IdSet;
	QFile lk_File(ls_IdListPath);
	if (!lk_File.open(QIODevice::ReadOnly))
	{
		printf("Error: Unable to open %s.\n", ls_IdListPath.toStdString().c_str());
		exit(1);
	}
	
	QTextStream lk_Stream(&lk_File);
	while (!lk_Stream.atEnd())
	{
		QString ls_Id = lk_Stream.readLine();
		if (!ls_Id.isEmpty())
			lk_IdSet.insert(ls_Id);
	}
	lk_File.close();
	
	k_SpectrumExtractor lk_Extractor;
	lk_Extractor.extract(ls_SpotPath, lk_IdSet);
}
