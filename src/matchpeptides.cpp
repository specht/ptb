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
#include <ptb/FastaReader.h>
#include <ptb/RefPtr.h>
#include "version.h"


struct r_PeptideInProtein
{
	int mi_PeptideStart;
	int mi_PeptideLength;
	int mi_ProteinLength;
	QString ms_Left;
	QString ms_Right;
};

// protein => occurences
typedef QHash<QString, QList<r_PeptideInProtein> > tk_PeptideInProteinList;


QHash<QString, tk_PeptideInProteinList> lk_Peptides;

void printUsageAndExit()
{
	printf("Usage: matchpeptides [options] --peptides [peptides] --peptideFiles [peptide files] --modelFiles [model files]\n");
	printf("Options:\n");
	printf("  --output [path] (default: stdout)\n");
	printf("      Redirect YAML output to a file.\n");
	printf("  --borderSize [int] (default: 5)\n");
	printf("      Define how many additional amino acids should be printed for each hit.\n");
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
	QStringList lk_Arguments;
	for (int i = 1; i < ai_ArgumentCount; ++i)
		lk_Arguments << ac_Arguments__[i];
		
	if (!lk_Arguments.empty() && (lk_Arguments.first() == "--version"))
	{
		printf("matchpeptides %s\n", gs_Version.toStdString().c_str());
		exit(0);
	}
		
	RefPtr<QFile> lk_pYamlOutFile;
	
	QFile lk_StdOut;
	lk_StdOut.open(stdout, QIODevice::WriteOnly);
	QIODevice* lk_YamlDevice_ = &lk_StdOut;
	
	// consume options
	int li_Index;
	int li_BorderSize = 5;
	
	li_Index = lk_Arguments.indexOf("--output");
	if (li_Index > -1)
	{
		lk_pYamlOutFile = RefPtr<QFile>(new QFile(lk_Arguments[li_Index + 1]));
		lk_pYamlOutFile->open(QIODevice::WriteOnly);
		lk_YamlDevice_ = lk_pYamlOutFile.get_Pointer();
		lk_Arguments.removeAt(li_Index);
		lk_Arguments.removeAt(li_Index);
	}
	
	li_Index = lk_Arguments.indexOf("--borderSize");
	if (li_Index > -1)
	{
		lk_Arguments.removeAt(li_Index);
		li_BorderSize = QVariant(lk_Arguments[li_Index]).toInt();
		lk_Arguments.removeAt(li_Index);
	}
	
	QSet<QString> lk_PeptidesSet;
	QStringList lk_ModelFiles;
	while (!lk_Arguments.empty())
	{
		if (lk_Arguments.first() == "--peptides")
		{
			lk_Arguments.removeFirst();
			while (!lk_Arguments.empty() && !lk_Arguments.first().startsWith("-"))
				lk_PeptidesSet.insert(lk_Arguments.takeFirst().toUpper());
		}
		else if (lk_Arguments.first() == "--peptideFiles")
		{
			lk_Arguments.removeFirst();
			while (!lk_Arguments.empty() && !lk_Arguments.first().startsWith("-"))
			{	
				QString ls_Path = lk_Arguments.takeFirst();
				QFile lk_File(ls_Path);
				if (!lk_File.open(QIODevice::ReadOnly))
				{
					printf("Error: Unable to open %s.\n", ls_Path.toStdString().c_str());
					exit(1);
				}
				QTextStream lk_Stream(&lk_File);
				while (!lk_Stream.atEnd())
				{
					QString ls_Line = lk_Stream.readLine().trimmed();
					if (!ls_Line.startsWith(">"))
						lk_PeptidesSet.insert(ls_Line.toUpper());
				}
			}
		}
		else if (lk_Arguments.first() == "--modelFiles")
		{
			lk_Arguments.removeFirst();
			while (!lk_Arguments.empty() && !lk_Arguments.first().startsWith("-"))
				lk_ModelFiles << lk_Arguments.takeFirst();
		}
		else
		{
			printf("Error: Unknown command line switch: %s\n", lk_Arguments.first().toStdString().c_str());
			exit(1);
		}
	}
	
	if (lk_ModelFiles.empty() || lk_PeptidesSet.empty())
		printUsageAndExit();
		
	QHash<QString, tk_PeptideInProteinList> lk_Peptides;
	// remove duplicate peptides
	foreach (QString ls_Peptide, lk_PeptidesSet.toList())
		lk_Peptides[ls_Peptide] = tk_PeptideInProteinList();
	
	// try to assign proteins to peptides
	foreach (QString ls_Path, lk_ModelFiles)
	{
		k_FastaReader lk_FastaReader(ls_Path);
		r_FastaEntry lr_Entry;
		while (lk_FastaReader.readEntry(&lr_Entry))
		{
			lr_Entry.ms_Entry = lr_Entry.ms_Entry.toUpper();
			foreach (QString ls_Peptide, lk_Peptides.keys())
			{
				int li_Index = lr_Entry.ms_Entry.indexOf(ls_Peptide);
				if (li_Index > -1)
				{
					r_PeptideInProtein lr_PeptideInProtein;
					lr_PeptideInProtein.mi_PeptideStart = li_Index;
					lr_PeptideInProtein.mi_PeptideLength = ls_Peptide.length();
					lr_PeptideInProtein.mi_ProteinLength = lr_Entry.ms_Entry.length();
					int li_LeftStart = std::max<int>(0, li_Index - li_BorderSize);
					lr_PeptideInProtein.ms_Left = lr_Entry.ms_Entry.mid(li_LeftStart, li_Index - li_LeftStart);
					lr_PeptideInProtein.ms_Right = lr_Entry.ms_Entry.mid(li_Index + ls_Peptide.length(), li_BorderSize);
					if (!lk_Peptides[ls_Peptide].contains(lr_Entry.ms_Id))
						lk_Peptides[ls_Peptide][lr_Entry.ms_Id] = QList<r_PeptideInProtein>();
					lk_Peptides[ls_Peptide][lr_Entry.ms_Id].push_back(lr_PeptideInProtein);
				}
			}
		}
	}
	
	QTextStream lk_Stream(lk_YamlDevice_);
	foreach (QString ls_Peptide, lk_Peptides.keys())
	{
		lk_Stream << ls_Peptide << ":" << endl;
		foreach (QString ls_Protein, lk_Peptides[ls_Peptide].keys())
		{
			lk_Stream << "  \"" << ls_Protein << "\":" << endl;
			foreach (r_PeptideInProtein lr_PeptideInProtein, lk_Peptides[ls_Peptide][ls_Protein])
				lk_Stream << "    - { start: " << lr_PeptideInProtein.mi_PeptideStart << 
					", length: " << lr_PeptideInProtein.mi_PeptideLength <<
					", proteinLength: " << lr_PeptideInProtein.mi_ProteinLength << 
					", left: \"" << lr_PeptideInProtein.ms_Left << "\"" <<
					", right: \"" << lr_PeptideInProtein.ms_Right << "\"" <<
					" }" << endl;
		}
	}
}
