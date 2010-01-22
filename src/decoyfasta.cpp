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


struct r_DecoyMethod
{
	enum Enumeration
	{
		Shuffle,
		Reverse,
		ShufflePeptide,
	};
};


void printUsageAndExit()
{
	printf("Usage: decoyfasta [options] [fasta files]\n");
	printf("Options:\n");
	printf("  --method [shuffle|reverse|shuffle-peptide] (default: reverse)\n");
	printf("      specify how a decoy entry should be built\n");
	printf("      note: the --keepStart and --keepEnd parameters have no effect\n");
	printf("      if shuffle-peptide is chosen here\n");
	printf("  --keepStart [int] (default: 0)\n");
	printf("      specify how many amino acids/nucleotides on the left side of \n");
	printf("      an entry should be left untouched.\n");
	printf("  --keepEnd [int] (default: 1)\n");
	printf("      specify how many amino acids/nucleotides on the right side of \n");
	printf("      an entry should be left untouched.\n");
	printf("  --targetFormat [string] (default: 'target_')\n");
	printf("      Specify how target ids should be generated.\n");
	printf("  --decoyFormat [string] (default: 'decoy_')\n");
	printf("      Specify how decoy ids should be generated.\n");
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


bool sortByFilename(const QString& a, const QString& b)
{
	return QFileInfo(a).fileName().toLower() < QFileInfo(b).fileName().toLower();
}

QString shufflePeptide(QString as_Peptide)
{
	QString ls_Result;
	QStringList lk_UntouchedSiteStrings;
	lk_UntouchedSiteStrings << "K" << "R" << "KP" << "RP" << "*";
	// ls_Mask contains a star for every character that is to be left untouched
	// and a space for every char which is to be shuffled
	QString ls_Mask;
	for (int i = 0; i < as_Peptide.length(); ++i)
		ls_Mask += " ";
	foreach (QString ls_SiteString, lk_UntouchedSiteStrings)
	{
		int li_Index = as_Peptide.indexOf(ls_SiteString);
		while (li_Index != -1)
		{
			for (int i = 0; i < ls_SiteString.length(); ++i)
				ls_Mask[i + li_Index] = QChar('*');
			li_Index = as_Peptide.indexOf(ls_SiteString, li_Index + 1);
		}
	}
	
	// fill result with all 'to be shuffled' chars
	for (int i = 0; i < as_Peptide.length(); ++i)
	{
		if (ls_Mask.at(i) != QChar('*'))
			ls_Result += as_Peptide.at(i);
	}
	
	QString ls_TempResult;
	// shuffle result into ls_TempResult, clear ls_Result on the way
	while (!ls_Result.isEmpty())
	{
		int li_Index = rand() % ls_Result.size();
		QChar lc_Char = ls_Result[li_Index];
		ls_Result[li_Index] = ls_Result[ls_Result.size() - 1];
		ls_Result.truncate(ls_Result.size() - 1);
		ls_TempResult += lc_Char;
	}
	
	ls_Result.clear();
	
	// construct final result by merging untouched chars with shuffled amino acids
	for (int i = 0; i < as_Peptide.length(); ++i)
	{
		if (ls_Mask.at(i) == QChar('*'))
			ls_Result += as_Peptide.at(i);
		else
		{
			ls_Result += ls_TempResult.at(0);
			ls_TempResult = ls_TempResult.right(ls_TempResult.length() - 1);
		}
	}
	
	return ls_Result;
}


QString makeDecoy(QString as_Entry, r_DecoyMethod::Enumeration ae_Method)
{
	QString ls_Result;
	if (ae_Method == r_DecoyMethod::Reverse)
	{
		for (int i = as_Entry.size() - 1; i >= 0; --i)
			ls_Result += as_Entry[i];
	}
	else if (ae_Method == r_DecoyMethod::ShufflePeptide)
	{
		// perform in silico digest of the protein (tryptic cleavage for now, more to come)
		int li_Index = 0;
		QString ls_Peptide;
		while (li_Index < as_Entry.length())
		{
			ls_Peptide += as_Entry.mid(li_Index, 1);
			if ((as_Entry.mid(li_Index, 1) == "R" || as_Entry.mid(li_Index, 1) == "K") &&
				(!((as_Entry.mid(li_Index, 2) == "RP") || (as_Entry.mid(li_Index, 2) == "KP"))))
			{
				ls_Result += shufflePeptide(ls_Peptide);
				ls_Peptide.clear();
			}
			++li_Index;
		}
		if (!ls_Peptide.isEmpty())
			ls_Result += shufflePeptide(ls_Peptide);
	}
	else
	{
		while (!as_Entry.isEmpty())
		{
			int li_Index = rand() % as_Entry.size();
			QChar lc_Char = as_Entry[li_Index];
			as_Entry[li_Index] = as_Entry[as_Entry.size() - 1];
			as_Entry.truncate(as_Entry.size() - 1);
			ls_Result += lc_Char;
		}
	}
    // swap every K and R with its predecessor
    /*
    for (int i = 1; i < ls_Result.length(); ++i)
    {
        if (ls_Result[i] == 'K' || ls_Result[i] == 'R')
        {
            QChar lc_Temp = ls_Result.at(i);
            ls_Result[i] = ls_Result[i - 1];
            ls_Result[i - 1] = lc_Temp;
        }
    }
    */
	return ls_Result;
}


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
	srand(666); // oy, lucky number!
	
	QStringList lk_Arguments;
	for (int i = 1; i < ai_ArgumentCount; ++i)
		lk_Arguments << ac_Arguments__[i];
		
	if (!lk_Arguments.empty() && (lk_Arguments.first() == "--version"))
	{
		printf("decoyfasta %s\n", gs_Version.toStdString().c_str());
		exit(0);
	}
		
	QFile lk_OutputFile;
	lk_OutputFile.open(stdout, QIODevice::WriteOnly);
	
	r_DecoyMethod::Enumeration le_Method = r_DecoyMethod::Reverse;
	int li_KeepStart = 0;
	int li_KeepEnd = 1;
	QString ls_TargetFormat = "target_%1";
	QString ls_DecoyFormat = "decoy_%1";
	
	// consume options
	while (!lk_Arguments.empty())
	{
		if (lk_Arguments.first() == "-o" || lk_Arguments.first() == "--output")
		{
			lk_Arguments.removeFirst();
			lk_OutputFile.close();
			lk_OutputFile.setFileName(lk_Arguments.takeFirst());
			lk_OutputFile.open(QIODevice::WriteOnly);
		} 
		else if (lk_Arguments.first() == "--method")
		{
			lk_Arguments.removeFirst();
			QString ls_Method = lk_Arguments.takeFirst();
			if (ls_Method == "shuffle")
				le_Method = r_DecoyMethod::Shuffle;
			else if (ls_Method == "reverse")
				le_Method = r_DecoyMethod::Reverse;
			else if (ls_Method == "shuffle-peptide")
				le_Method = r_DecoyMethod::ShufflePeptide;
			else
			{
				printf("Error: Unknown target-decoy method '%s'.\n", ls_Method.toStdString().c_str());
				exit(1);
			}
		} 
		else if (lk_Arguments.first() == "--targetFormat")
		{
			lk_Arguments.removeFirst();
			ls_TargetFormat = lk_Arguments.takeFirst() + "%1";
		} 
		else if (lk_Arguments.first() == "--decoyFormat")
		{
			lk_Arguments.removeFirst();
			ls_DecoyFormat = lk_Arguments.takeFirst() + "%1";
		} 
		else if (lk_Arguments.first() == "--keepStart")
		{
			lk_Arguments.removeFirst();
			bool lb_Ok = false;
			QString ls_Value = lk_Arguments.takeFirst();
			li_KeepStart = QVariant(ls_Value).toInt(&lb_Ok);
			if (!lb_Ok)
			{
				printf("Error: Invalid integer '%s'.\n", ls_Value.toStdString().c_str());
				exit(1);
			}
		} 
		else if (lk_Arguments.first() == "--keepEnd")
		{
			lk_Arguments.removeFirst();
			bool lb_Ok = false;
			QString ls_Value = lk_Arguments.takeFirst();
			li_KeepEnd = QVariant(ls_Value).toInt(&lb_Ok);
			if (!lb_Ok)
			{
				printf("Error: Invalid integer '%s'.\n", ls_Value.toStdString().c_str());
				exit(1);
			}
		} 
		else
			break;
	}
	
	if (le_Method == r_DecoyMethod::ShufflePeptide)
	{
		li_KeepStart = 0;
		li_KeepEnd = 0;
	}
	
	QStringList lk_FastaFiles;
	while (!lk_Arguments.empty() && !lk_Arguments.first().startsWith("-"))
		lk_FastaFiles << lk_Arguments.takeFirst();
		
	if (lk_FastaFiles.empty())
		printUsageAndExit();
		
	k_FastaWriter lk_FastaWriter(&lk_OutputFile);
	
	// sort input files by filename, so that a certain mix of input files
	// always results in exactly the same target-decoy database, regardless
	// of the order in which the input files were specified
	
	qSort(lk_FastaFiles.begin(), lk_FastaFiles.end(), sortByFilename);
	
	foreach (QString ls_Path, lk_FastaFiles)
	{
		k_FastaReader lk_FastaReader(ls_Path);
		r_FastaEntry lr_Entry;
		while (lk_FastaReader.readEntry(&lr_Entry))
		{
			r_FastaEntry lr_TargetEntry = lr_Entry;
			lr_TargetEntry.ms_Id = QString(ls_TargetFormat).arg(lr_TargetEntry.ms_Id);
			lk_FastaWriter.writeEntry(lr_TargetEntry);
			r_FastaEntry lr_DecoyEntry = lr_Entry;
			lr_DecoyEntry.ms_Id = QString(ls_DecoyFormat).arg(lr_DecoyEntry.ms_Id);
			// create decoy
			if (lr_DecoyEntry.ms_Entry.length() > li_KeepStart + li_KeepEnd)
			{
				// only change the entry if there's something left to shuffle/reverse
				// after the untouched start and end fragments have been left alone
				QString ls_Start = lr_DecoyEntry.ms_Entry.left(li_KeepStart);
				QString ls_End = lr_DecoyEntry.ms_Entry.right(li_KeepEnd);
				lr_DecoyEntry.ms_Entry.truncate(lr_DecoyEntry.ms_Entry.length() - li_KeepEnd);
				lr_DecoyEntry.ms_Entry.remove(0, li_KeepStart);
				lr_DecoyEntry.ms_Entry = ls_Start + makeDecoy(lr_DecoyEntry.ms_Entry, le_Method) + ls_End;
			}
			lk_FastaWriter.writeEntry(lr_DecoyEntry);
		}
	}
	
	lk_OutputFile.close();
}
