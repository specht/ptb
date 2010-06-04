/*
Copyright (c) 2009 Michael Specht

This file is part of the Proteomics Toolbox.

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
#include "Yaml.h"


typedef QSet<int> tk_IntSet;
typedef QPair<tk_IntSet, tk_IntSet> tk_IntSetPair;
typedef QList<int> tk_IntList;
typedef QPair<tk_IntSet, tk_IntList> tk_IntSetIntListPair;
    
    
void printUsageAndExit()
{
    printf("Usage: groupproteins [options] [infile]\n");
    printf("Options:\n");
    printf("  --output [path] (default: stdout)\n");
    printf("      Redirect YAML output to a file.\n");
    printf("  --version\n");
    printf("      Print version and exit.\n");
    printf("Input format (YAML):\n");
    printf("peptides: [A, B, C]\n");
    printf("proteins: [1, 2, 3]\n");
    printf("peptidesForProtein: {0: [0, 1], 1: [1]}\n");

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
        printf("Error: unknown boolean value '%s'.\n", as_String.toStdString().c_str());
        exit(1);
    }
};


QHash<int, QSet<int> > lk_PeptidesForProtein;


bool compareByPeptideCount(const int a, const int b)
{
    return lk_PeptidesForProtein[b].size() < lk_PeptidesForProtein[a].size();
}


QList<tk_IntSetPair> lk_ProteinGroups;


bool compareByGroupPeptideCount(const int a, const int b)
{
    return lk_ProteinGroups[b].first.size() < lk_ProteinGroups[a].first.size();
}


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    QStringList lk_Arguments;
    for (int i = 1; i < ai_ArgumentCount; ++i)
        lk_Arguments << ac_Arguments__[i];
        
    if (!lk_Arguments.empty() && (lk_Arguments.first() == "--version"))
    {
        printf("groupproteins %s\n", gs_Version.toStdString().c_str());
        exit(0);
    }
        
    QSharedPointer<QFile> lk_pYamlOutFile;
    
    QFile lk_StdOut;
    lk_StdOut.open(stdout, QIODevice::WriteOnly);
    QIODevice* lk_YamlDevice_ = &lk_StdOut;
    
    if (lk_Arguments.empty() || lk_Arguments.first() == "--help")
        printUsageAndExit();
    
    // consume options
    int li_Index;
    
    li_Index = lk_Arguments.indexOf("--output");
    if (li_Index > -1)
    {
        lk_pYamlOutFile = QSharedPointer<QFile>(new QFile(lk_Arguments[li_Index + 1]));
        lk_pYamlOutFile->open(QIODevice::WriteOnly);
        lk_YamlDevice_ = lk_pYamlOutFile.data();
        lk_Arguments.removeAt(li_Index);
        lk_Arguments.removeAt(li_Index);
    }
    
    if (lk_Arguments.empty())
        printUsageAndExit();
    
    QString ls_Path = lk_Arguments.takeFirst();
    
    if (!QFileInfo(ls_Path).exists())
    {
        printf("Error: Unable to open %s.\n", ls_Path.toStdString().c_str());
        exit(1);
    }
    
    tk_YamlMap lk_Info = k_Yaml::parseFromFile(ls_Path).toMap();
    if (!(lk_Info.contains("peptides") && lk_Info.contains("proteins") && lk_Info.contains("peptidesForProtein")))
    {
        printf("Error: Input file does not contain peptide and protein information!\n");
        exit(1);
    }

    // load peptides
    QStringList lk_Peptides;
    foreach (QVariant lk_Peptide, lk_Info["peptides"].toList())
        lk_Peptides << lk_Peptide.toString();
    
    QStringList lk_ProteinNames;
    foreach (QVariant ls_Protein, lk_Info["proteins"].toList())
        lk_ProteinNames << ls_Protein.toString();
    
    QHash<int, int> lk_PeptideOccurences;
    
    // load proteins
    tk_YamlMap lk_ProteinInfo = lk_Info["peptidesForProtein"].toMap();
    foreach (QString ls_Protein, lk_ProteinInfo.keys())
    {
        int li_ProteinIndex = ls_Protein.toInt();
        lk_PeptidesForProtein[li_ProteinIndex] = QSet<int>();
        foreach (QVariant lk_PeptideIndex, lk_ProteinInfo[ls_Protein].toList())
        {
            int li_PeptideIndex = lk_PeptideIndex.toInt();
            lk_PeptidesForProtein[li_ProteinIndex] << li_PeptideIndex;
            if (!lk_PeptideOccurences.contains(li_PeptideIndex))
                lk_PeptideOccurences[li_PeptideIndex] = 0;
            lk_PeptideOccurences[li_PeptideIndex] += 1;
        }
    }
    
    int li_UnambiguousPeptideCount = 0;
    foreach (int li_PeptideIndex, lk_PeptideOccurences.keys())
    {
        if (lk_PeptideOccurences[li_PeptideIndex] == 1)
            li_UnambiguousPeptideCount += 1;
    }
    
    printf("Peptide yield before grouping: %1.1f%% of %d peptides.\n", 
           (double)li_UnambiguousPeptideCount * 100.0 / lk_PeptideOccurences.size(),
           lk_PeptideOccurences.size());
    
    int li_TotalProteinCount = lk_ProteinNames.size();
    
    QSet<int> lk_SingletonProteins;
    for (int a = 0; a < li_TotalProteinCount; ++a)
        lk_SingletonProteins << a;
    
    QList<tk_IntSetIntListPair> lk_ProteinGroupsSorted;
    
    int li_TotalIterationCount = (li_TotalProteinCount * li_TotalProteinCount - li_TotalProteinCount) / 2;
    int li_IterationOffset = 0;
    QString ls_LastPercentage;
    
    for (int a = 0; a < li_TotalProteinCount; ++a)
    {
        for (int b = a + 1; b < li_TotalProteinCount; ++b)
        {
            if (li_IterationOffset % 1000 == 0)
            {
                QString ls_Percentage = QString("%1").arg((double)((double)li_IterationOffset * 100.0 / li_TotalIterationCount), 1, 'f', 1);
                if (ls_Percentage != ls_LastPercentage)
                {
                    printf("\rAnalyzing protein pairs... %s%% done.", ls_Percentage.toStdString().c_str());
                    ls_LastPercentage = ls_Percentage;
                }
            }
            ++li_IterationOffset;
            tk_IntSet lk_PeptidesA = lk_PeptidesForProtein[a];
            tk_IntSet lk_PeptidesB = lk_PeptidesForProtein[b];
            tk_IntSet lk_Combined = lk_PeptidesA | lk_PeptidesB;
            int li_MaxSize = qMax(lk_PeptidesA.size(), lk_PeptidesB.size());
            if (lk_Combined.size() == li_MaxSize)
            {
                // these proteins belong together!
                lk_SingletonProteins.remove(a);
                lk_SingletonProteins.remove(b);
                bool lb_CouldJoin = false;
                for (int i = 0; i < lk_ProteinGroups.size(); ++i)
                {
                    tk_IntSetPair lk_Pair = lk_ProteinGroups[i];
                    tk_IntSet lk_Peptides = lk_Pair.first;
                    tk_IntSet lk_SubMerged = lk_Peptides | lk_Combined;
                    int li_MaxSize = qMax(lk_Peptides.size(), lk_Combined.size());
                    if (lk_SubMerged.size() == li_MaxSize)
                    {
                        // we can join an existing protein group!
                        lk_ProteinGroups[i].first |= lk_Combined;
                        lk_ProteinGroups[i].second << a;
                        lk_ProteinGroups[i].second << b;
                        lb_CouldJoin = true;
                        break;
                    }
                }
                if (!lb_CouldJoin)
                    // start a new protein group
                    lk_ProteinGroups.append(tk_IntSetPair(lk_Combined, tk_IntSet() << a << b));
            }
        }
    }
    printf("\rAnalyzing protein pairs... 100.0%% done.\n");
    
    // add ungrouped proteins
    foreach (int a, lk_SingletonProteins)
        lk_ProteinGroups.append(tk_IntSetPair(lk_PeptidesForProtein[a], tk_IntSet() << a));
    
    // convert protein sets into sorted protein lists, 
    // all-peptide-comprising ones first
    for (int i = 0; i < lk_ProteinGroups.size(); ++i)
    {
        tk_IntSetPair lk_InPair = lk_ProteinGroups[i];
        tk_IntSetIntListPair lk_OutPair;
        lk_OutPair.first = lk_InPair.first;
        foreach (int li_Peptide, lk_InPair.second)
            lk_OutPair.second << li_Peptide;
        qSort(lk_OutPair.second.begin(), lk_OutPair.second.end(), &compareByPeptideCount);
        lk_ProteinGroupsSorted << lk_OutPair;
    }
    
    printf("There are %d protein groups.\n", lk_ProteinGroups.size());

    // determine unique and razor peptides
    QHash<int, tk_IntSet> lk_GroupsForPeptide;
    for (int i = 0; i < lk_ProteinGroups.size(); ++i)
    {
        foreach (int li_PeptideIndex, lk_ProteinGroups[i].first)
        {
            if (!lk_GroupsForPeptide.contains(li_PeptideIndex))
                lk_GroupsForPeptide[li_PeptideIndex] = tk_IntSet();
            lk_GroupsForPeptide[li_PeptideIndex] << i;
        }
    }
    
    QTextStream lk_Stream(lk_YamlDevice_);
    lk_Stream << "---\n";
    lk_Stream << "proteinGroups:\n";
    foreach (tk_IntSetIntListPair lk_Pair, lk_ProteinGroupsSorted)
    {
        bool lb_First = true;
        foreach (int li_ProteinIndex, lk_Pair.second)
        {
            if (lb_First)
                lk_Stream << "- ";
            else
                lk_Stream << "  ";
            lb_First = false;
            
            lk_Stream << "- \"" << lk_ProteinNames[li_ProteinIndex].replace("\"", "\\\"") << "\"\n";
        }
    }
    
    int li_UniquePeptideCount = 0;
    
    lk_Stream << "peptides:\n";
    foreach (int li_PeptideIndex, lk_GroupsForPeptide.keys())
    {
        QList<int> lk_GroupList;
        foreach (int li_GroupIndex, lk_GroupsForPeptide[li_PeptideIndex])
            lk_GroupList << li_GroupIndex;
        qSort(lk_GroupList.begin(), lk_GroupList.end(), &compareByGroupPeptideCount);
        if (lk_GroupList.size() == 1)
            ++li_UniquePeptideCount;
        lk_Stream << "  " << lk_Peptides[li_PeptideIndex] << ": [";
        for (int i = 0; i < lk_GroupList.size(); ++i)
        {
            if (i != 0)
                lk_Stream << ", ";
            lk_Stream << lk_GroupList[i];
        }
        lk_Stream << "]\n";
    }
    
    printf("Peptide yield after grouping: %1.1f%% of %d peptides.\n", 
           (double)li_UniquePeptideCount * 100.0 / lk_GroupsForPeptide.size(),
           lk_GroupsForPeptide.size());
}
