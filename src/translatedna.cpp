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
#include <ptb/FastaReader.h>
#include <ptb/FastaWriter.h>
#include "version.h"


void printUsageAndExit()
{
	printf("Usage: translatedna [options] [fasta file]\n");
	printf("Options:\n");
	printf("  --frames [int list] (default: 1,2,3,4,5,6)\n");
	printf("      specify which frames should be translated.\n");
	printf("  --headerFormat [string] (default: 'ORF_%%1_%%2_frame%%3')\n");
	printf("      specify header format, where %%1 is basename,\n");
	printf("      %%2 is scaffold id and %%3 is the frame.\n");
	printf("  -o, --output [filename]: specify output filename (default: stdout)\n");
	printf("  --version\n");
	printf("      Print version and exit.\n");
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
	Q_INIT_RESOURCE(ptb);
	
	QHash<QString, QChar> lk_TripletToAminoAcid;
	
	QFile lk_File(":/res/DnaToAminoAcid.csv");
	if (!lk_File.open(QIODevice::ReadOnly))
	{
		printf("Internal error: Unable to open amino acid code wheel file!\n");
		exit(1);
	}
	QTextStream lk_Stream(&lk_File);
	while (!lk_Stream.atEnd())
	{
		QStringList lk_Line = lk_Stream.readLine().trimmed().split(";");
		lk_TripletToAminoAcid[lk_Line.first()] = lk_Line.last().at(0);
	}
	lk_File.close();
	
	QStringList lk_Arguments;
	for (int i = 1; i < ai_ArgumentCount; ++i)
		lk_Arguments << ac_Arguments__[i];
		
	if (!lk_Arguments.empty() && (lk_Arguments.first() == "--version"))
	{
		printf("translatedna %s\n", gs_Version.toStdString().c_str());
		exit(0);
	}
		
	QFile lk_OutputFile;
	lk_OutputFile.open(stdout, QIODevice::WriteOnly);
	QString ls_FramesToTranslate = "1,2,3,4,5,6";
	QString ls_HeaderFormat = "ORF_%1_%2_frame%3";
	
	// consume options
	while (!lk_Arguments.empty())
	{
		if (lk_Arguments.first() == "-o" || lk_Arguments.first() == "--output")
		{
			lk_Arguments.removeFirst();
			lk_OutputFile.close();
			lk_OutputFile.setFileName(lk_Arguments.takeFirst());
			if (!lk_OutputFile.open(QIODevice::WriteOnly))
			{
				printf("Error: Unable to open %s for writing.\n", lk_OutputFile.fileName().toStdString().c_str());
				exit(1);
			}
		} 
		else if (lk_Arguments.first() == "--frames")
		{
			lk_Arguments.removeFirst();
			ls_FramesToTranslate = lk_Arguments.takeFirst();
		} 
		else if (lk_Arguments.first() == "--headerFormat")
		{
			lk_Arguments.removeFirst();
			ls_HeaderFormat = lk_Arguments.takeFirst();
		}
		else
			break;
	}

	QStringList lk_FastaFiles;
	while (!lk_Arguments.empty() && !lk_Arguments.first().startsWith("-"))
		lk_FastaFiles << lk_Arguments.takeFirst();
	
	QHash<QChar, QChar> lk_Transpose;
	lk_Transpose[QChar('A')] = QChar('U');
	lk_Transpose[QChar('C')] = QChar('G');
	lk_Transpose[QChar('G')] = QChar('C');
	lk_Transpose[QChar('U')] = QChar('A');
	
	QSet<int> lk_FramesToTranslate;
	foreach (QString ls_Id, ls_FramesToTranslate.split(","))
	{
		int li_Id = QVariant(ls_Id).toInt();
		if (li_Id < 1 || li_Id > 6)
		{
			printf("Error: Invalid frame id: %d.\n", li_Id);
			exit(1);
		}
		lk_FramesToTranslate.insert(li_Id - 1);
	}
		
	if (lk_FastaFiles.empty())
		printUsageAndExit();
		
	k_FastaWriter lk_FastaWriter(&lk_OutputFile);
	
	foreach (QString ls_Path, lk_FastaFiles)
	{
		QString ls_Organism = QFileInfo(ls_Path).baseName();
		k_FastaReader lk_FastaReader(ls_Path);
		r_FastaEntry lr_Entry;
		while (lk_FastaReader.readEntry(&lr_Entry))
		{
			lr_Entry.ms_Entry.replace("T", "U");
			for (int li_Direction = 0; li_Direction < 2; ++li_Direction)
			{
				if (li_Direction == 1)
				{
					// no reverse method in QString!? DIY!
					QString ls_Reversed;
					for (int i = lr_Entry.ms_Entry.length() - 1; i >= 0; --i)
					{
						QChar lc_Char = lr_Entry.ms_Entry.at(i);
						if (lk_Transpose.contains(lc_Char))
							lc_Char = lk_Transpose[lc_Char];
						ls_Reversed += lc_Char;
					}
					lr_Entry.ms_Entry = ls_Reversed;
				}
				for (int li_Frame = 0; li_Frame < 3; ++li_Frame)
				{
					// skip this frame if not wanted
					if (!lk_FramesToTranslate.contains(li_Direction * 3 + li_Frame))
						continue;
					
					r_FastaEntry lr_TargetEntry;
					lr_TargetEntry.ms_Id = QString(ls_HeaderFormat).arg(ls_Organism).arg(lr_Entry.ms_Id).arg(li_Direction * 3 + li_Frame + 1);
					int li_Offset = li_Frame;
					while (li_Offset + 2 < lr_Entry.ms_Entry.length())
					{
						QString ls_Triplet = lr_Entry.ms_Entry.mid(li_Offset, 3);
						if (!lk_TripletToAminoAcid.contains(ls_Triplet))
						{
							printf("Error: Invalid nucleotide triplet: '%s', %d, %d.\n", ls_Triplet.toStdString().c_str(), li_Offset, lr_Entry.ms_Entry.length());
							exit(1);
						}
						lr_TargetEntry.ms_Entry += lk_TripletToAminoAcid[ls_Triplet];
						li_Offset += 3;
					}
					//printf("%d %s\n", lr_TargetEntry.ms_Entry.length(), lr_TargetEntry.ms_Entry.toStdString().c_str());
					lk_FastaWriter.writeEntry(lr_TargetEntry);
				}
			}
		}
	}
	
	lk_OutputFile.close();
}
