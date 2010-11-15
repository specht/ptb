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

#include <ptb/PeakMatcher.h>
#include <QtCore>


k_PeakMatcher::k_PeakMatcher(r_ScanType::Enumeration ae_ScanType,
                             QList<tk_IntPair> ak_MsLevels)
    : k_ScanIterator(ae_ScanType, ak_MsLevels)
{
/*    mk_TextStream.setRealNumberNotation(QTextStream::FixedNotation);
    mk_TextStream.setRealNumberPrecision(10);
    mk_TextStream.setNumberFlags(QTextStream::ForcePoint);*/
}


k_PeakMatcher::~k_PeakMatcher()
{
}


void k_PeakMatcher::match(QStringList ak_SpectraFiles, 
                          QIODevice* ak_OutDevice_,
                          QStringList ak_Targets,
                          QSet<int> ak_MsLevels,
                          double ad_MassAccuracy,
                          double ad_Snr,
                          double ad_Crop
                         )
{
    mk_pOutStream = QSharedPointer<QTextStream>(new QTextStream(ak_OutDevice_));
    mk_MsLevels = ak_MsLevels;
    md_MassAccuracy = ad_MassAccuracy;
    md_Snr = ad_Snr;
    md_Crop = ad_Crop;

    (*mk_pOutStream) << QString("Filename,Scan ID,MS level,Match count,Peak m/z,Peak intensity,Target m/z,Mass difference (peak-target),Mass error (ppm)\n");

    mk_TargetStrings.clear();
    mk_TargetMz.clear();
    mk_TargetMzMin.clear();
    mk_TargetMzMax.clear();
    QMap<double, int> lk_TargetIndex;
    for (int i = 0; i < ak_Targets.size(); ++i)
    {
        QString ls_Target = ak_Targets[i];
        bool lb_Ok = false;
        double ld_Mz = ls_Target.toDouble(&lb_Ok);
        if (!lb_Ok)
        {
            fprintf(stderr, "Error: Invalid m/z value '%s'.\n", ls_Target.toStdString().c_str());
            exit(1);
        }
        if (ld_Mz < 0.0)
        {
            fprintf(stderr, "Error: Invalid m/z value: %s.\n", ls_Target.toStdString().c_str());
            exit(1);
        }
        if (lk_TargetIndex.contains(ld_Mz))
        {
            fprintf(stderr, "Error: m/z value specified multiple times: %s.\n", ls_Target.toStdString().c_str());
            exit(1);
        }
        lk_TargetIndex.insert(ld_Mz, i);
    }
    foreach (double ld_Mz, lk_TargetIndex.keys())
    {
        int li_Index = lk_TargetIndex[ld_Mz];
        mk_TargetStrings << ak_Targets[li_Index];
        bool lb_Ok = false;
        double ld_Mz = ak_Targets[li_Index].toDouble(&lb_Ok);
        mk_TargetMz << ld_Mz;
        double ld_Error = ld_Mz * md_MassAccuracy / 1000000.0;
        mk_TargetMzMin << ld_Mz - ld_Error;
        mk_TargetMzMax << ld_Mz + ld_Error;
    }
    
    foreach (QString ls_Path, ak_SpectraFiles)
    {
        ms_SpotName = QFileInfo(ls_Path).baseName();
        this->parseFile(ls_Path);
    }
    fprintf(stderr, "\n");
}


void k_PeakMatcher::handleScan(r_Scan& ar_Scan, bool& ab_Continue)
{
    ab_Continue = true;
    if (!mk_MsLevels.contains(ar_Scan.mi_MsLevel))
        return;
    
    fprintf(stderr, "\r%s: scan #%s", 
            ms_SpotName.toStdString().c_str(), 
            ar_Scan.ms_Id.toStdString().c_str());
    if (ar_Scan.mr_Spectrum.mi_PeaksCount == 0)
    {
        fprintf(stderr, "Warning: Empty spectrum (scan #%s @ %1.2f minutes)!\n", ar_Scan.ms_Id.toStdString().c_str(), ar_Scan.md_RetentionTime);
        return;
    }
    
    QList<r_Peak> lk_Peaks = findAllPeaks(ar_Scan.mr_Spectrum, md_Snr);
    
    // now chuck out all peaks which are less than md_Crop relative to
    // the maximum intensity
    // 1. determine maximum intensity
    double ld_MaxIntensity = 0.0;
    foreach (r_Peak lr_Peak, lk_Peaks)
        if (lr_Peak.md_PeakIntensity > ld_MaxIntensity)
            ld_MaxIntensity = lr_Peak.md_PeakIntensity;
    // collect a list of indices to be deleted
    QList<int> lk_DeleteIndices;
    for (int i = 0; i < lk_Peaks.size(); ++i)
    {
        r_Peak lr_Peak = lk_Peaks[i];
        if (lr_Peak.md_PeakIntensity < ld_MaxIntensity * md_Crop)
            lk_DeleteIndices << i;
    }
    #ifdef DEBUF
    qDebug() << "deleting" <<  lk_DeleteIndices.size() << "indices."
    #endif
    // delete indices in reversed order
    for (int i = lk_DeleteIndices.size() - 1; i >= 0; --i)
        lk_Peaks.removeAt(lk_DeleteIndices[i]);
    // done and done!
    
    // now match observed peaks to target peaks, both lists are sorted
/*    for (int i = 0; i < ar_Scan.mr_Spectrum.mi_PeaksCount; ++i)
        fprintf(stderr, "%1.4f\n", ar_Scan.mr_Spectrum.md_MzValues_[i]);*/
    int li_TargetIndex = 0;
    int li_PeakIndex = 0;
    while (true)
    {
        // exit the loop if there are no peaks or targets left
        if (li_TargetIndex >= mk_TargetMz.size())
            break;
        if (li_PeakIndex >= lk_Peaks.size())
            break;

        /*
        fprintf(stderr, "P %9.4f / T %9.4f (%9.4f .. %9.4f)\n", 
                ar_Scan.mr_Spectrum.md_MzValues_[li_PeakIndex],
                mk_TargetMz[li_TargetIndex],
                mk_TargetMzMin[li_TargetIndex],
                mk_TargetMzMax[li_TargetIndex]
               );
        */
        
        // advance peak if left of current target minimum
        if (lk_Peaks[li_PeakIndex].md_PeakMz < mk_TargetMzMin[li_TargetIndex])
        {
            ++li_PeakIndex;
//             fprintf(stderr, "+peak\n");
            continue;
        }
        
        // advance target if left of current peak
        if (lk_Peaks[li_PeakIndex].md_PeakMz > mk_TargetMzMax[li_TargetIndex])
        {
            ++li_TargetIndex;
//             fprintf(stderr, "+target\n");
            continue;
        }
        
        
        int li_TryTargetIndex = li_TargetIndex;
        QList<int> lk_Matches;
        while (mk_TargetMzMin[li_TryTargetIndex] <= lk_Peaks[li_PeakIndex].md_PeakMz &&
            mk_TargetMzMax[li_TryTargetIndex] >= lk_Peaks[li_PeakIndex].md_PeakMz)
        {
            lk_Matches << li_TryTargetIndex;
            ++li_TryTargetIndex;
            if (li_TryTargetIndex >= mk_TargetMz.size())
                break;
        }
        foreach (int li_MatchTargetIndex, lk_Matches)
        {
            double ld_MassDiff = lk_Peaks[li_PeakIndex].md_PeakMz - mk_TargetMz[li_MatchTargetIndex];
            double ld_MassError = (fabs(ld_MassDiff) / mk_TargetMz[li_MatchTargetIndex]) * 1000000.0;
            (*mk_pOutStream) << QString("\"%1\",%2,%3,%4,%5,%6,%7,%8,%9\n")
                .arg(ms_SpotName)
                .arg(ar_Scan.ms_Id)
                .arg(ar_Scan.mi_MsLevel)
                .arg(lk_Matches.size())
                .arg(lk_Peaks[li_PeakIndex].md_PeakMz, 0, 'f', 4)
                .arg(lk_Peaks[li_PeakIndex].md_PeakIntensity, 0, 'f', 4)
                .arg(mk_TargetStrings[li_MatchTargetIndex])
                .arg(ld_MassDiff, 0, 'f', 4)
                .arg(ld_MassError, 0, 'f', 4);
        }
        ++li_PeakIndex;
    }
}


QString k_PeakMatcher::ftos(double ad_Value, int ai_DecimalPlaces)
{
    int li_DecimalPlaces = 6;
    if (ai_DecimalPlaces >= 0)
        li_DecimalPlaces = ai_DecimalPlaces;
    QString ls_Result = QString("%1").arg(ad_Value, 0, 'f', li_DecimalPlaces);
    if (ls_Result.contains("."))
    {
        while (ls_Result.endsWith("0"))
            ls_Result.truncate(ls_Result.length() - 1);
        if (ls_Result.endsWith("."))
            ls_Result.truncate(ls_Result.length() - 1);
    }
    return ls_Result;
}
