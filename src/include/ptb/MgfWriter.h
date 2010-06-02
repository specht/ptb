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


class k_MgfWriter: public k_ScanIterator
{
public:
    k_MgfWriter(r_ScanType::Enumeration ae_ScanType = r_ScanType::All,
                QList<tk_IntPair> ak_MsLevels = QList<tk_IntPair>() << tk_IntPair(0, 0x10000));
    virtual ~k_MgfWriter();
    
    virtual void convert(QStringList ak_SpectraFiles, QString as_OutputPath, 
                         int ai_BatchSize = 0, 
                         QString as_RetentionTimesPath = QString(),
                         QSet<QString> ak_Ids = QSet<QString>());
    virtual void handleScan(r_Scan& ar_Scan, bool& ab_Continue);
    
protected:
    virtual void flushScan(r_Scan& ar_Scan, r_Precursor* ar_Precursor_ = NULL);
    virtual QString ftos(double ad_Value);
    
    QTextStream mk_TextStream;
    QSharedPointer<QFile> mk_pFile;
    QFile* mk_RetentionTimesFile_;
    QTextStream* mk_RetentionTimesStream_;
    bool mb_UseInputCharges;
    QString ms_SpotName;
    QString ms_OutputPath;
    int mi_BatchSize;
    int mi_PartCounter;
    int mi_CurrentBatchSize;
    QSet<QString> mk_Ids;
    QSet<QString> mk_FoundIds;
};
