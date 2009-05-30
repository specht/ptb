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
#include <ptb/RefPtr.h>
#include "version.h"
#include <ptb/ZipFileOrNot.h>


void printUsageAndExit()
{
	printf("Usage: stripscans [in path]\n");
	printf("This tool will strip all MS1 scans from the input file.\n");
	printf("Spectra file must be mzML, optionally compressed (.gz, .bz2, .zip).\n");
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
		printf("stripscans %s\n", gs_Version.toStdString().c_str());
		exit(0);
	}
	
	if (lk_Arguments.empty())
		printUsageAndExit();
		
	QString ls_InputPath = lk_Arguments.first();
	QString ls_OutputPath = QFileInfo(ls_InputPath).baseName() + "-no-ms1.mzML";
	
	k_ZipFileOrNot lk_File(ls_InputPath);
	lk_File.device()->open(QIODevice::ReadOnly);
	
	QFile lk_OutFile(ls_OutputPath);
	if (lk_OutFile.exists())
	{
		printf("Error: %s already exists. I won't overwrite this file.\n", ls_OutputPath.toStdString().c_str());
		exit(1);
	}
	
	lk_OutFile.open(QIODevice::WriteOnly);
	QTextStream lk_OutStream(&lk_OutFile);
	
	QString ls_SpectrumString;
	int li_IgnoreThis = 0;
	while (!lk_File.device()->atEnd())
	{
		QString ls_Line = lk_File.device()->readLine();
		if (ls_Line.trimmed().startsWith("<indexedmzML"))
			continue;
		if (ls_Line.trimmed().startsWith("<indexListOffset>"))
			continue;
		if (ls_Line.trimmed().startsWith("<fileChecksum>"))
			continue;
		if (ls_Line.trimmed().startsWith("</indexedmzML"))
			continue;
		if (ls_Line.trimmed().startsWith("<chromatogramList "))
			++li_IgnoreThis;
		if (ls_Line.trimmed().startsWith("</chromatogramList>"))
		{
			--li_IgnoreThis;
			continue;
		}
		if (ls_Line.trimmed().startsWith("<indexList "))
			++li_IgnoreThis;
		if (ls_Line.trimmed().startsWith("</indexList>"))
		{
			--li_IgnoreThis;
			continue;
		}
		if (li_IgnoreThis > 0)
			continue;
		if (ls_Line.trimmed().startsWith("<spectrum "))
			ls_SpectrumString += ls_Line + "\n";
		else if (ls_Line.trimmed().startsWith("</spectrum>"))
		{
			ls_SpectrumString += ls_Line + "\n";
			// determine scan ms level
			int li_Index = ls_SpectrumString.indexOf("MS:1000511");
			if (li_Index > 0)
			{
				int li_Start = ls_SpectrumString.lastIndexOf("<cvParam", li_Index);
				int li_End = ls_SpectrumString.indexOf(">", li_Index);
				QString ls_CvParam = ls_SpectrumString.mid(li_Start, li_End - li_Start + 1);
				QRegExp lk_RegExp("(value=\")(.+)(\")");
				if (lk_RegExp.indexIn(ls_CvParam) > -1)
				{
					QString ls_Level = lk_RegExp.cap(2);
					int li_Level = ls_Level.toInt();
					if (li_Level > 1)
						lk_OutStream << ls_SpectrumString;
				}
			}
			ls_SpectrumString.clear();
		}
		else
		{
			if (ls_SpectrumString.isEmpty())
			{
				if (ls_Line.trimmed().startsWith("<spectrumList "))
					ls_Line = "      <spectrumList>";
				lk_OutStream << ls_Line << endl;
			}
			else
				ls_SpectrumString += ls_Line + "\n";
		}
	}
	
	lk_OutStream.flush();
	lk_OutFile.close();
}
