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

#include "SpectrumChecker.h"
#include <QtCore>
    

k_SpectrumChecker::k_SpectrumChecker()
{
}


k_SpectrumChecker::~k_SpectrumChecker()
{
}


void k_SpectrumChecker::check(QStringList ak_SpectraFiles)
{
    foreach (QString ls_Path, ak_SpectraFiles)
    {
        ms_CurrentSpot = QFileInfo(ls_Path).baseName();
        mk_ScanCounter.clear();
        mi_WarningCounter = 0;
        mi_ScanCounter = 0;
        printf("%s: ", QFileInfo(ls_Path).fileName().toStdString().c_str());
        this->parseFile(ls_Path);
        printf("found %d scans", mi_ScanCounter);
        if (mi_WarningCounter == 0)
            printf(", all ok.\n", mi_ScanCounter);
        else
            printf(", %d warnings.\n", mi_WarningCounter);
            
        foreach (QString ls_Key, mk_ScanCounter.keys())
            printf("%s: %i scans\n", ls_Key.toStdString().c_str(), mk_ScanCounter[ls_Key]);
    }
}


void k_SpectrumChecker::handleScan(r_Scan& ar_Scan, bool& ab_Continue)
{
    ab_Continue = true;
    ++mi_ScanCounter;
    if (ar_Scan.mr_Spectrum.mi_PeaksCount == 0)
    {
        if (mi_WarningCounter == 0)
            printf("\n");
        ++mi_WarningCounter;
        printf("Warning: Empty spectrum #%s @ %1.2f minutes.\n", ar_Scan.ms_Id.toStdString().c_str(), ar_Scan.md_RetentionTime);
    }
    QString ls_Type = "unknown";
    switch (ar_Scan.me_Type)
    {
    case r_ScanType::MS1: ls_Type = "MS1"; break;
    case r_ScanType::MSn: ls_Type = "MSn"; break;
    case r_ScanType::CRM: ls_Type = "CRM"; break;
    case r_ScanType::SIM: ls_Type = "SIM"; break;
    case r_ScanType::SRM: ls_Type = "SRM"; break;
    case r_ScanType::PDA: ls_Type = "PDA"; break;
    case r_ScanType::SICC: ls_Type = "SICC"; break;
    }
    
    QString ls_Description = QString("%1 scan (MS%2)").arg(ls_Type).arg(ar_Scan.mi_MsLevel);
    if (!mk_ScanCounter.contains(ls_Description))
        mk_ScanCounter[ls_Description] = 0;
    ++mk_ScanCounter[ls_Description];
}


void k_SpectrumChecker::progressFunction(QString as_ScanId, bool ab_InterestingScan)
{
    printf("\r%s: scan #%s...", ms_CurrentSpot.toStdString().c_str(), as_ScanId.toStdString().c_str());
}
