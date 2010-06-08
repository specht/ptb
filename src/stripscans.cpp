/*
Copyright (c) 2007-2010 Michael Specht

This file is part of Proteomics Toolbox.

Proteomics Toolbox is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Proteomics Toolbox is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Proteomics Toolbox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtCore>
#include <stdio.h>
#include "version.h"
#include <ptb/ZipFileOrNot.h>


void printUsageAndExit()
{
    fprintf(stderr, "Usage: stripscans [options] [spectra file]\n");
    fprintf(stderr, "This tool strips scans from the input mzML file.\n");
    fprintf(stderr, "Spectra file must be mzML, optionally compressed (.gz, .bz2, .zip).\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --outputPath <string> (default: stdout)\n");
    fprintf(stderr, "      Specify the output path.\n");
    fprintf(stderr, "  --stripMsLevels <string> (default: empty)\n");
    fprintf(stderr, "      Specify MS levels of scans to strip from the input file, separated\n");
    fprintf(stderr, "      by comma or whitespace (example: '1,2').\n");
    fprintf(stderr, "  --stripScanIds <path> (optional)\n");
    fprintf(stderr, "      Strip all scans specified in a text file located at <path>.\n");
    fprintf(stderr, "      Cannot be used together with --keepScanIds.\n");
    fprintf(stderr, "  --keepScanIds <path> (optional)\n");
    fprintf(stderr, "      Keep all scans specified in a text file located at <path>.\n");
    fprintf(stderr, "      Cannot be used together with --stripScanIds.\n");
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
        fprintf(stderr, "Error: unknown boolean value '%s'.\n", as_String.toStdString().c_str());
        exit(1);
    }
};


QSet<int> readEntriesFromFile(const QString& as_Path)
{
    QSet<int> lk_Results;
    QFile lk_File(as_Path);
    if (!lk_File.open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "Error: Unable to open %s.\n", as_Path.toStdString().c_str());
        exit(1);
    }
    QTextStream lk_Stream(&lk_File);
    int li_LineNumber = 0;
    while (!lk_Stream.atEnd())
    {
        QString ls_Line = lk_Stream.readLine().trimmed();
        if (ls_Line.isEmpty())
            continue;
        ++li_LineNumber;
        bool lb_Ok = false;
        int li_Id = ls_Line.toInt(&lb_Ok);
        if (!lb_Ok)
        {
            fprintf(stderr, "Error: Invalid scan id in %s:%d.\n", as_Path.toStdString().c_str(), li_LineNumber);
            exit(1);
        }
        lk_Results << li_Id;
    }
    lk_File.close();
    return lk_Results;
}


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    QStringList lk_Arguments;
    for (int i = 1; i < ai_ArgumentCount; ++i)
        lk_Arguments << ac_Arguments__[i];
        
    if (!lk_Arguments.empty() && (lk_Arguments.first() == "--version"))
    {
        fprintf(stderr, "stripscans %s\n", gs_Version.toStdString().c_str());
        exit(0);
    }
    
    if (lk_Arguments.empty())
        printUsageAndExit();

    QSharedPointer<QFile> lk_pOutputFile;
    QTextStream lk_OutStream(stdout);
    
    QSet<int> lk_StripMsLevels;
    QSet<int> lk_ScanIds;
    bool lb_StripIds = false;
    bool lb_KeepIds = false;
    
    // parse arguments
    while (!lk_Arguments.empty())
    {
        QString ls_Argument = lk_Arguments.takeFirst();
        if (ls_Argument == "--stripMsLevels")
        {
            if (lk_Arguments.empty())
            {
                fprintf(stderr, "Error: No MS scan level to strip have been specified after --stripMsLevels.\n");
                exit(1);
            }
            QRegExp lk_RegExp("[\\s,]+");
            foreach (QString ls_Level, lk_Arguments.takeFirst().split(lk_RegExp, QString::SkipEmptyParts))
            {
                bool lb_Ok = false;
                int li_Level = ls_Level.toInt(&lb_Ok);
                if (!lb_Ok)
                {
                    fprintf(stderr, "Error: Invalid MS scan level specified (%s).\n", ls_Level.toStdString().c_str());
                    exit(1);
                }
                lk_StripMsLevels << li_Level;
            }
        }
        else if (ls_Argument == "--keepScanIds")
        {
            lb_KeepIds = true;
            if (lk_Arguments.empty())
            {
                fprintf(stderr, "Error: No filename specified after --keepScanIds.\n");
                exit(1);
            }
            lk_ScanIds = readEntriesFromFile(lk_Arguments.takeFirst());
        }
        else if (ls_Argument == "--stripScanIds")
        {
            lb_StripIds = true;
            if (lk_Arguments.empty())
            {
                fprintf(stderr, "Error: No filename specified after --stripScanIds.\n");
                exit(1);
            }
            lk_ScanIds = readEntriesFromFile(lk_Arguments.takeFirst());
        }
        else if (ls_Argument == "--outputPath")
        {
            QString ls_Path = lk_Arguments.takeFirst();
            if (lk_Arguments.empty())
            {
                fprintf(stderr, "Error: No filename specified after --outputPath.\n");
                exit(1);
            }
            lk_pOutputFile = QSharedPointer<QFile>(new QFile(ls_Path));
            if (!lk_pOutputFile->open(QIODevice::WriteOnly))
            {
                fprintf(stderr, "Error: Unable to open output file for writing.\n");
                exit(1);
            }
            lk_OutStream.setDevice(lk_pOutputFile.data());
        }
        else
        {
            lk_Arguments.insert(0, ls_Argument); // PUSH argument back, it must be a filename
            break;
        }
    }

    if (lb_KeepIds && lb_StripIds)
    {
        fprintf(stderr, "Error: --keepScanIds and --stripScanIds must not be specified together.\n");
        exit(1);
    }
    
    if (!lk_StripMsLevels.empty())
    {
        QList<int> lk_Levels = lk_StripMsLevels.toList();
        qSort(lk_Levels);
        fprintf(stderr, "Stripping MS level%s ",
               lk_StripMsLevels.size() == 1 ? "" : "s");
        foreach (int li_Level, lk_Levels)
        {
            if (li_Level != lk_Levels.first())
            {
                if (li_Level == lk_Levels.last())
                    fprintf(stderr, " and ");
                else
                    fprintf(stderr, ", ");
            }
            fprintf(stderr, "%d", li_Level);
        }
        fprintf(stderr, ".\n");
    }
    if (lb_KeepIds || lb_StripIds)
    {
        fprintf(stderr, lb_KeepIds ? "Keeping" : "Stripping");
        fprintf(stderr, " %d scan IDs.\n", lk_ScanIds.size());
    }

    if (lk_Arguments.empty())
    {
        fprintf(stderr, "Error: No input filename specified.\n");
        exit(1);
    }
    QString ls_InputPath = lk_Arguments.first();

    k_ZipFileOrNot lk_File(ls_InputPath);
    if (!lk_File.device()->open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "Error: Unable to open %s.\n", ls_InputPath.toStdString().c_str());
        exit(1);
    }
    
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
            ls_SpectrumString += ls_Line;
        else if (ls_Line.trimmed().startsWith("</spectrum>"))
        {
            ls_SpectrumString += ls_Line;
            // determine scan ms level
            bool lb_PrintThisSpectrum = true;
            // reject certain MS levels
            int li_Index = ls_SpectrumString.indexOf("MS:1000511");
            if (li_Index > 0)
            {
                int li_Start = ls_SpectrumString.lastIndexOf("<cvParam", li_Index);
                int li_End = ls_SpectrumString.indexOf(">", li_Index);
                QString ls_CvParam = ls_SpectrumString.mid(li_Start, li_End - li_Start + 1);
                QRegExp lk_RegExp("(value=\")([^\"]+)(\")");
                if (lk_RegExp.indexIn(ls_CvParam) > -1)
                {
                    QString ls_Level = lk_RegExp.cap(2);
                    int li_Level = ls_Level.toInt();
                    if (lk_StripMsLevels.contains(li_Level))
                        lb_PrintThisSpectrum = false;
                }
            }
            if (lb_KeepIds || lb_StripIds)
            {
                lb_PrintThisSpectrum = lb_StripIds;
                // keep or strip certain scan IDs
                QRegExp lk_RegExp("(nativeID=\")([^\"]+)(\")");
                if (lk_RegExp.indexIn(ls_SpectrumString) > -1)
                {
                    QString ls_Id = lk_RegExp.cap(2);
                    if (lk_ScanIds.contains(ls_Id.toInt()))
                        lb_PrintThisSpectrum = !lb_PrintThisSpectrum;
                }
                else
                {
                    // no nativeID in mzML 1.1.0+
                    QRegExp lk_RegExp("(id=\")([^\"]+)(\")");
                    if (lk_RegExp.indexIn(ls_SpectrumString) > -1)
                    {
                        QString ls_IdContainer = lk_RegExp.cap(2);
                        QStringList lk_Id = ls_IdContainer.split(" ");
                        foreach (QString ls_Pair, lk_Id)
                        {
                            QStringList lk_Pair = ls_Pair.split("=");
                            if (lk_Pair[0].trimmed() == "scan")
                            {
                                QString ls_Id = lk_Pair[1].trimmed();
                                if (lk_ScanIds.contains(ls_Id.toInt()))
                                    lb_PrintThisSpectrum = !lb_PrintThisSpectrum;
                            }
                        }
                    }
                }
            }
            if (lb_PrintThisSpectrum)
                lk_OutStream << ls_SpectrumString;
            ls_SpectrumString.clear();
        }
        else
        {
            if (ls_SpectrumString.isEmpty())
            {
                if (ls_Line.trimmed().startsWith("<spectrumList "))
                    ls_Line = "      <spectrumList>\n";
                lk_OutStream << ls_Line;;
            }
            else
                ls_SpectrumString += ls_Line;
        }
    }
    
    lk_OutStream.flush();
}
