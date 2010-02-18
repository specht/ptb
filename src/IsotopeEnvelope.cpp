/*
Copyright (c) 2009-2010 Michael Specht

This file is part of qTrace.

qTrace is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SimQuant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with qTrace.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "include/ptb/IsotopeEnvelope.h"


k_IsotopeEnvelope::k_IsotopeEnvelope(tk_ModifiedAbundances ak_ModifiedAbundances)
{
    Q_INIT_RESOURCE(ptb);
    
    QFile lk_File(":ext/proteomics-knowledge-base/isotopes.csv");
    if (!lk_File.open(QFile::ReadOnly))
    {
        fprintf(stderr, "Error: Unable to open isotopes.csv.\n");
        exit(1);
    }
    QTextStream lk_TextStream(&lk_File);
    QString ls_Header = lk_TextStream.readLine().toLower().trimmed();
    QStringList lk_Header = ls_Header.split(",");
    int li_ElementIndex = lk_Header.indexOf("element");
    int li_IsotopeIndex = lk_Header.indexOf("isotope");
    int li_MassIndex = lk_Header.indexOf("monoisotopic mass");
    int li_AbundanceIndex = lk_Header.indexOf("natural abundance");
    if (li_ElementIndex < 0 || li_IsotopeIndex < 0 || li_MassIndex < 0 || li_AbundanceIndex < 0)
    {
        fprintf(stderr, "Error: Something is wrong with isotopes.csv.\n");
        exit(1);
    }
    while (!lk_TextStream.atEnd())
    {
        QString ls_Line = lk_TextStream.readLine().trimmed();
        if (ls_Line.isEmpty())
            continue;
        QStringList lk_Line = ls_Line.split(",");
        QString ls_Element = lk_Line[li_ElementIndex];
        int li_Isotope = QVariant(lk_Line[li_IsotopeIndex]).toInt();
        double ld_Mass = QVariant(lk_Line[li_MassIndex]).toDouble();
        double ld_Abundance = QVariant(lk_Line[li_AbundanceIndex]).toDouble();
        if (!mk_BaseIsotope.contains(ls_Element))
            mk_BaseIsotope[ls_Element] = li_Isotope;
        if (!mk_BaseIsotopeMass.contains(ls_Element))
            mk_BaseIsotopeMass[ls_Element] = ld_Mass;
        if (!mk_ElementEnvelopes.contains(ls_Element))
            mk_ElementEnvelopes[ls_Element] = QList<tk_IsotopeEnvelope>() << tk_IsotopeEnvelope();
        if (!mk_ElementIsotopeMassShift.contains(ls_Element))
            mk_ElementIsotopeMassShift[ls_Element] = QList<double>();
        int li_RelativeIsotope = li_Isotope - mk_BaseIsotope[ls_Element];
        // override natural abundance if desired
        if (ak_ModifiedAbundances.contains(ls_Element))
            ld_Abundance = ak_ModifiedAbundances[ls_Element][li_RelativeIsotope];
        while (li_RelativeIsotope != mk_ElementEnvelopes[ls_Element].first().size())
        {
            // in the list, an isotope was skipped, fill with 0.0
            mk_ElementEnvelopes[ls_Element].first() << QPair<double, double>(0.0, 0.0);
            mk_ElementIsotopeMassShift[ls_Element] << 0.0;
        }
        double ld_MassShift = ld_Mass - mk_BaseIsotopeMass[ls_Element];
        mk_ElementEnvelopes[ls_Element].first() << QPair<double, double>(ld_Abundance, ld_MassShift);
        mk_ElementIsotopeMassShift[ls_Element] << ld_MassShift;
    }
}


k_IsotopeEnvelope::~k_IsotopeEnvelope()
{
}


tk_IsotopeEnvelope 
k_IsotopeEnvelope::isotopeEnvelopeForComposition(QHash<QString, int> ak_Composition)
{
    // update cached element isotope envelopes
    foreach (QString ls_Element, ak_Composition.keys())
    {
        int li_Index = 1;
        int li_Count = 2;
        while (li_Count <= ak_Composition[ls_Element])
        {
            if (li_Index >= mk_ElementEnvelopes[ls_Element].size())
            {
                // create isotope envelope for li_Count atoms of ls_Element
                // by adding isotope envelopes for (li_Count / 2) atoms
                mk_ElementEnvelopes[ls_Element] << add(mk_ElementEnvelopes[ls_Element].last(), mk_ElementEnvelopes[ls_Element].last());
            }
            ++li_Index;
            li_Count <<= 1;
        }
    }
    
    tk_IsotopeEnvelope lk_Result;
    bool lb_First = true;
    foreach (QString ls_Element, ak_Composition.keys())
    {
        int li_Count = ak_Composition[ls_Element];
        int li_CacheIndex = 0;
        while (li_Count > 0)
        {
            if ((li_Count & 1) == 1)
            {
                if (lb_First)
                {
                    lk_Result = mk_ElementEnvelopes[ls_Element][li_CacheIndex];
                    lb_First = false;
                }
                else
                    lk_Result = add(lk_Result, mk_ElementEnvelopes[ls_Element][li_CacheIndex]);
            }
            ++li_CacheIndex;
            li_Count >>= 1;
        }
    }
    
    // normalize maximum to 1.0
/*    double ld_Maximum = 0.0;
    typedef QPair<double, double> tk_DoublePair;
    foreach (tk_DoublePair lk_Pair, lk_Result)
        if (lk_Pair.first > ld_Maximum)
            ld_Maximum = lk_Pair.first;
        
    for (int i = 0; i < lk_Result.size(); ++i)
        lk_Result[i].first /= ld_Maximum;
    
    while ((!lk_Result.empty()) && (lk_Result.last().second < 0.001))
        lk_Result.takeLast();*/
    
    return lk_Result;
}


tk_IsotopeEnvelope k_IsotopeEnvelope::add(const tk_IsotopeEnvelope& ak_First, 
                                          const tk_IsotopeEnvelope& ak_Second)
{
    tk_IsotopeEnvelope lk_Result;
    
    int li_NewLength = ak_First.size() + ak_Second.size() - 1;
    for (int i = 0; i < li_NewLength; ++i)
        lk_Result << QPair<double, double>(0.0, 0.0);
    
    // do n * m iterations
    for (int i = 0; i < ak_First.size(); ++i)
    {
        for (int j = 0; j < ak_Second.size(); ++j)
        {
            double ld_ThisFactor = ak_First[i].first * ak_Second[j].first;
            lk_Result[i + j].first += ld_ThisFactor;
            lk_Result[i + j].second += ld_ThisFactor * (ak_First[i].second + ak_Second[j].second);
        }
    }
    
    // normalize mass shifts
    for (int i = 0; i < li_NewLength; ++i)
    {
        if (lk_Result[i].first == 0.0)
            lk_Result[i].second = 0.0;
        else
            lk_Result[i].second /= lk_Result[i].first;
    }
    
    return lk_Result;
}


tk_IsotopeEnvelope k_IsotopeEnvelope::normalize(const tk_IsotopeEnvelope& ak_Envelope)
{
    tk_IsotopeEnvelope lk_Result = ak_Envelope;
    double ld_Maximum = 0.0;
    for (int i = 0; i < lk_Result.size(); ++i)
        ld_Maximum = std::max<double>(ld_Maximum, lk_Result[i].first);
    
    for (int i = 0; i < lk_Result.size(); ++i)
        lk_Result[i].first /= ld_Maximum;
    
    return lk_Result;
}


double k_IsotopeEnvelope::massForComposition(QHash<QString, int> ak_Composition)
{
    double ld_Mass = 0.0;
    foreach (QString ls_Element, ak_Composition.keys())
        ld_Mass += mk_BaseIsotopeMass[ls_Element] * ak_Composition[ls_Element];
    return ld_Mass;
}
