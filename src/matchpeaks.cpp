/*
Copyright (c) 2010 Michael Specht

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
#include <ptb/PeakMatcher.h>
#include "version.h"


void printUsageAndExit()
{
    fprintf(stderr, "Usage: matchpeaks [options] [target m/z (.txt)] [spectra files]\n");
    fprintf(stderr, "Spectra files may be mzML, mzXML or mzData, optionally compressed (.gz, .bz2, .zip).\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -a, --accuracy [float]: specify mass accuracy in ppm (default: 5.0 ppm)\n");
    fprintf(stderr, "  -s, --snr [float]: specify signal-to-noise ratio for peak picking (default: 2.0)\n");
    fprintf(stderr, "  -c, --crop [float]: specify minimum peak relative abundance (default: 0.05)\n");
    fprintf(stderr, "  -l, --levels [list]: specify MS levels to be matched (default: all)\n");
    fprintf(stderr, "    examples: '1', '1,2', or 'all'\n");
    fprintf(stderr, "  -o, --output [filename]: specify output filename\n");
    exit(1);
}


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    QStringList lk_Arguments;
    for (int i = 1; i < ai_ArgumentCount; ++i)
        lk_Arguments << ac_Arguments__[i];
        
    if (lk_Arguments.empty())
        printUsageAndExit();
        
    QSharedPointer<QFile> lk_pOutFile;
    QFile lk_StdOut;
    lk_StdOut.open(stdout, QIODevice::WriteOnly);
    QIODevice* lk_OutDevice_ = &lk_StdOut;

    double ld_MassAccuracy = 5.0;
    double ld_Snr = 2.0;
    double ld_Crop = 0.05;
    QString ls_Levels = "all";
    
    // consume options
    while (!lk_Arguments.empty())
    {
        if (lk_Arguments.first() == "-a" || lk_Arguments.first() == "--accuracy")
        {
            lk_Arguments.removeFirst();
            QString ls_Accuracy = lk_Arguments.takeFirst();
            bool lb_Ok = false;
            ld_MassAccuracy = ls_Accuracy.toDouble(&lb_Ok);
            if (!lb_Ok)
            {
                fprintf(stderr, "Error: Invalid mass accuracy specified: '%s'.\n", ls_Accuracy.toStdString().c_str());
                exit(1);
            }
        } 
        else if (lk_Arguments.first() == "-s" || lk_Arguments.first() == "--snr")
        {
            lk_Arguments.removeFirst();
            QString ls_Snr = lk_Arguments.takeFirst();
            bool lb_Ok = false;
            ld_Snr = ls_Snr.toDouble(&lb_Ok);
            if (!lb_Ok)
            {
                fprintf(stderr, "Error: Invalid signal-to-noise threshold specified: '%s'.\n", ls_Snr.toStdString().c_str());
                exit(1);
            }
        } 
        else if (lk_Arguments.first() == "-c" || lk_Arguments.first() == "--crop")
        {
            lk_Arguments.removeFirst();
            QString ls_Crop = lk_Arguments.takeFirst();
            bool lb_Ok = false;
            ld_Crop = ls_Crop.toDouble(&lb_Ok);
            if (!lb_Ok)
            {
                fprintf(stderr, "Error: Invalid crop threshold specified: '%s'.\n", ls_Crop.toStdString().c_str());
                exit(1);
            }
        } 
        else if (lk_Arguments.first() == "-o" || lk_Arguments.first() == "--output")
        {
            lk_Arguments.removeFirst();
            QString ls_OutputPath = lk_Arguments.takeFirst();
            lk_pOutFile = QSharedPointer<QFile>(new QFile(ls_OutputPath));
            lk_pOutFile->open(QIODevice::WriteOnly);
            lk_OutDevice_ = lk_pOutFile.data();
        } 
        else if (lk_Arguments.first() == "-l" || lk_Arguments.first() == "--levels")
        {
            lk_Arguments.removeFirst();
            ls_Levels = lk_Arguments.takeFirst();
        } 
        else if (lk_Arguments.first() == "--help")
        {
            lk_Arguments.removeFirst();
            printUsageAndExit();
        }
        else
            break;
    }

    QString ls_LevelsDescription = "all MS levels";
    QSet<int> lk_UseLevels;
    if (ls_Levels.toLower() == "all")
    {
        for (int i = 1; i < 10000; ++i)
            lk_UseLevels << i;
    }
    else
    {
        QStringList lk_Levels = ls_Levels.split(",");
        QList<int> lk_LevelsInt;
        foreach (QString ls_Level, lk_Levels)
        {
            bool lb_Ok = false;
            int li_Level = ls_Level.toInt(&lb_Ok);
            if (!lb_Ok)
            {
                fprintf(stderr, "Error: Invalid MS level specified: %s\n.", ls_Level.toStdString().c_str());
                exit(1);
            }
            if (li_Level < 1)
            {
                fprintf(stderr, "Error: Invalid MS level specified: %d.\n", li_Level);
                exit(1);
            }
            lk_UseLevels << li_Level;
            lk_LevelsInt << li_Level;
        }
        qSort(lk_LevelsInt);
        QStringList lk_Desc;
        foreach (int i, lk_LevelsInt)
            lk_Desc << QString("%1").arg(i);
        ls_LevelsDescription = QString() + "MS level" + ((lk_UseLevels.size() == 1) ? "" : "s") + " " + lk_Desc.join(",");
    }
    
    // read target m/z values
    QStringList lk_Targets;
    if (lk_Arguments.empty())
    {
        fprintf(stderr, "Error: No m/z target text file specified.\n");
        exit(1);
    }
    QString ls_TargetPath = lk_Arguments.takeFirst();
    QFile lk_TargetFile(ls_TargetPath);
    if (!lk_TargetFile.open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "Error: Unable to open %s.\n", ls_TargetPath.toStdString().c_str());
        exit(1);
    }
    QTextStream lk_TargetStream(&lk_TargetFile);
    while (!lk_TargetStream.atEnd())
    {
        QString ls_Line = lk_TargetStream.readLine().trimmed();
        if (ls_Line.isEmpty())
            continue;
        lk_Targets << ls_Line;
    }
    
    if (lk_Arguments.empty())
    {
        fprintf(stderr, "Error: No spectral files specified.\n");
        exit(1);
    }
    
    QStringList lk_SpectraFiles;
    foreach (QString ls_Path, lk_Arguments)
    {
        if (!QFile::exists(ls_Path))
        {
            fprintf(stderr, "Error: Unable to open spectral file %s.\n", ls_Path.toStdString().c_str());
            exit(1);
        }
        lk_SpectraFiles << ls_Path;
    }
    
    k_PeakMatcher lk_PeakMatcher(r_ScanType::All, QList<tk_IntPair>() << tk_IntPair(1, 0x10000));
    fprintf(stderr, "Searching for %d target m/z value%s in %d spectral file%s, checking %s using a mass accuracy of %1.2f ppm.\nUsing a SNR of %1.2f and a crop threshold of %1.2f%%.\n", 
            lk_Targets.size(), 
            lk_Targets.size() == 1 ? "" : "s",
            lk_SpectraFiles.size(),
            lk_SpectraFiles.size() == 1 ? "" : "s",
            ls_LevelsDescription.toStdString().c_str(),
            ld_MassAccuracy, ld_Snr, ld_Crop * 100.0
           );
    lk_PeakMatcher.match(lk_SpectraFiles, lk_OutDevice_, lk_Targets, lk_UseLevels, ld_MassAccuracy, ld_Snr, ld_Crop);
}
