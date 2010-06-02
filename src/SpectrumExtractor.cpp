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

#include "SpectrumExtractor.h"
#include <QtCore>
    

k_SpectrumExtractor::k_SpectrumExtractor()
{
}


k_SpectrumExtractor::~k_SpectrumExtractor()
{
}


void k_SpectrumExtractor::extract(QString as_SpotPath, QSet<QString> ak_IdSet)
{
    mk_IdSet = ak_IdSet;
    this->parseFile(as_SpotPath);
}


void k_SpectrumExtractor::handleScan(r_Scan& ar_Scan)
{
    if (ar_Scan.mr_Spectrum.mi_PeaksCount == 0)
    {
        printf("Warning: Empty spectrum #%s @ %1.2f minutes.\n", ar_Scan.ms_Id.toStdString().c_str(), ar_Scan.md_RetentionTime);
        return;
    }
    printf("got scan #%s.\n", ar_Scan.ms_Id.toStdString().c_str());
}


void k_SpectrumExtractor::progressFunction(QString as_ScanId, bool ab_InterestingScan)
{
}

bool k_SpectrumExtractor::isInterestingScan(r_Scan& ar_Scan)
{
    return mk_IdSet.contains(ar_Scan.ms_Id);
}
