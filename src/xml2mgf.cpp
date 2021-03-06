/*
Copyright (c) 2007-2008 Michael Specht

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
#include <ptb/MgfWriter.h>
#include "version.h"


void printUsageAndExit()
{
    printf("Usage: xml2mgf [options] [spectra files]\n");
    printf("Spectra files may be mzML, mzXML or mzData, optionally compressed (.gz, .bz2, .zip).\n");
    printf("Options:\n");
    printf("  -o, --output [filename]: specify output filename\n");
    printf("  -mzdp, --mzDecimalPlaces [n]: specify number of decimal places for m/z values\n");
    printf("  -idp, --intensityDecimalPlaces [n]: specify number of decimal places for intensity values\n");
    printf("  -b, --batch [n]: create batches of n spectra\n");
    printf("  -rt, --retentionTimes [filename]: specify an output filename for scan retention times\n");
    printf("  -i, --id [id list]: specify which scans to extract\n");
    printf("      Multiple IDs can be separated by spaces or commas.\n");
    exit(1);
}


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    QStringList lk_Arguments;
    for (int i = 1; i < ai_ArgumentCount; ++i)
        lk_Arguments << ac_Arguments__[i];
        
    if (lk_Arguments.empty())
        printUsageAndExit();
        
    QString ls_OutputPath = "";
    QString ls_RetentionTimesPath = "";
    int li_BatchSize = 0;
    int li_MzDecimalPlaces = -1;
    int li_IntensityDecimalPlaces = -1;
    
    QSet<QString> lk_Ids;
    
    // consume options
    while (!lk_Arguments.empty())
    {
        if (lk_Arguments.first() == "-o" || lk_Arguments.first() == "--output")
        {
            lk_Arguments.removeFirst();
            ls_OutputPath = lk_Arguments.takeFirst();
        } 
        else if (lk_Arguments.first() == "-mzdp" || lk_Arguments.first() == "--mzDecimalPlaces")
        {
            lk_Arguments.removeFirst();
            li_MzDecimalPlaces = QVariant(lk_Arguments.takeFirst()).toInt();
        }
        else if (lk_Arguments.first() == "-idp" || lk_Arguments.first() == "--intensityDecimalPlaces")
        {
            lk_Arguments.removeFirst();
            li_IntensityDecimalPlaces = QVariant(lk_Arguments.takeFirst()).toInt();
        }
        else if (lk_Arguments.first() == "-b" || lk_Arguments.first() == "--batch")
        {
            lk_Arguments.removeFirst();
            li_BatchSize = QVariant(lk_Arguments.takeFirst()).toInt();
        }
        else if (lk_Arguments.first() == "-rt" || lk_Arguments.first() == "--retentionTimes")
        {
            lk_Arguments.removeFirst();
            ls_RetentionTimesPath = lk_Arguments.takeFirst();
        } 
        else if (lk_Arguments.first() == "-i" || lk_Arguments.first() == "--id")
        {
            lk_Arguments.removeFirst();
            QString ls_Ids = lk_Arguments.takeFirst();
            lk_Ids = ls_Ids.split(QRegExp("[,\\s]+")).toSet();
        } 
        else if (lk_Arguments.first() == "--help")
        {
            lk_Arguments.removeFirst();
            printUsageAndExit();
        }
        else
            break;
    }
    
    k_MgfWriter lk_MgfWriter(r_ScanType::All, QList<tk_IntPair>() << tk_IntPair(2, 0x10000));
    
    if (ls_OutputPath.isEmpty())
    {
        printf("Error: no output file specified.\n");
        exit(1);
    }
    
    QStringList lk_SpectraFiles;
    foreach (QString ls_Path, lk_Arguments)
    {
        if (!QFile::exists(ls_Path))
        {
            printf("Error: %s could not be found.\n", ls_Path.toStdString().c_str());
            exit(1);
        }
        lk_SpectraFiles << ls_Path;
    }
        
    lk_MgfWriter.convert(lk_SpectraFiles, ls_OutputPath, li_BatchSize, 
                         li_MzDecimalPlaces, li_IntensityDecimalPlaces, 
                         ls_RetentionTimesPath, lk_Ids);
}
