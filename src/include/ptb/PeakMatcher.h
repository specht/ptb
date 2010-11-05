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

#pragma once
#include <QtCore>
#include <ptb/ScanIterator.h>


class k_PeakMatcher: public k_ScanIterator
{
public:
    k_PeakMatcher(r_ScanType::Enumeration ae_ScanType = r_ScanType::All,
                  QList<tk_IntPair> ak_MsLevels = QList<tk_IntPair>() << tk_IntPair(0, 0x10000));
    virtual ~k_PeakMatcher();
    
    virtual void match(QStringList ak_SpectraFiles, 
                       QIODevice* ak_OutDevice_,
                       QStringList ak_Targets,
                       QSet<int> ak_MsLevels,
                       double ad_MassAccuracy
                      );
    
    virtual void handleScan(r_Scan& ar_Scan, bool& ab_Continue);
    
protected:
    virtual QString ftos(double ad_Value, int ai_DecimalPlaces = -1);
    
    QSharedPointer<QTextStream> mk_pOutStream;
    QString ms_SpotName;
    QStringList mk_TargetStrings;
    QList<double> mk_TargetMz;
    QList<double> mk_TargetMzMin;
    QList<double> mk_TargetMzMax;
    QSet<int> mk_MsLevels;
    double md_MassAccuracy;
    QHash<QString, QPair<double, double> > mk_TargetRange;
};
