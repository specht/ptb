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

#pragma once
#include <QtCore>
#include <ptb/ScanIterator.h>


class k_SpectrumChecker: public k_ScanIterator
{
public:
    k_SpectrumChecker();
    virtual ~k_SpectrumChecker();
    
    virtual void check(QStringList ak_SpectraFiles);
    virtual void handleScan(r_Scan& ar_Scan);
    virtual void progressFunction(QString as_ScanId, bool ab_InterestingScan);
    
protected:
    int mi_WarningCounter;
    int mi_ScanCounter;
    QHash<QString, int> mk_ScanCounter;
    QString ms_CurrentSpot;
};
