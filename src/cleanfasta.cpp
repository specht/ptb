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

#include <QtCore>
#include "FastaReader.h"
#include "FastaWriter.h"
#include "version.h"


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
	k_FastaReader
	// 0:03 for Chlamy, 1.34% of all entries redundant
	//lk_FastaReader("/home/michael/Desktop/Chlre3_1.GeneCatalogProteins.6JUL06.fasta");
	// 2:16 for 'all plants', ~250MB memory footprint, 5.15% of all entries redundant
	lk_FastaReader("/home/michael/Desktop/nr-Plants_allProteins_NCBI.fasta");
	r_FastaEntry lr_Entry;
	QHash<QString, QString> lk_Entries;
	QHash<QString, QString> lk_ReverseEntries;
	typedef QPair<QString, int> tk_LocationInEntry;
	QHash<QString, QList<tk_LocationInEntry> > lk_Tags;
	QHash<QString, QStringList> lk_EntriesStartingOnTag;
	
	int li_TagLength = 5;
	
	// read entries from FASTA file, record starting tags
	while (lk_FastaReader.readEntry(&lr_Entry))
	{
		// skip if this entry was already seen
		if (lk_ReverseEntries.contains(lr_Entry.ms_Entry))
			continue;
		
		lk_Entries[lr_Entry.ms_Id] = lr_Entry.ms_Entry;
		lk_ReverseEntries[lr_Entry.ms_Entry] = lr_Entry.ms_Id;
		
		QString ls_Tag = lr_Entry.ms_Entry.left(li_TagLength);
		if (!lk_EntriesStartingOnTag.contains(ls_Tag))
			lk_EntriesStartingOnTag[ls_Tag] = QStringList();
		lk_EntriesStartingOnTag[ls_Tag] << lr_Entry.ms_Id;
	}

	// record all remaining tags that appeared as a starting tag
	foreach (QString ls_Id, lk_Entries.keys())
	{
		QString ls_Entry = lk_Entries[ls_Id];
		for (int i = 0; i <= ls_Entry.length() - li_TagLength; ++i)
		{
			QString ls_Tag = ls_Entry.mid(i, li_TagLength);
			if (lk_EntriesStartingOnTag.contains(ls_Tag))
			{
				if (!lk_Tags.contains(ls_Tag))
					lk_Tags[ls_Tag] = QList<tk_LocationInEntry>();
				lk_Tags[ls_Tag] << tk_LocationInEntry(ls_Id, i);
			}
		}
	}
	
	QHash<QString, QStringList> lk_Containing;
	QHash<QString, QStringList> lk_Contained;
	QSet<QString> lk_RemainingEntries = lk_Entries.keys().toSet();
	
	printf("searching for pairs where one entry is contained in the other\n");
	int li_PairCount = 0;
	int li_ComparisonCount = 0;
	foreach (QString ls_Tag, lk_EntriesStartingOnTag.keys())
	{
		// left entry starts at 0
		QStringList lk_LeftEntries = lk_EntriesStartingOnTag[ls_Tag];
		QList<tk_LocationInEntry> lk_RightEntries = lk_Tags[ls_Tag];
		foreach (QString ls_LeftId, lk_LeftEntries)
		{
			int li_LeftIdPairCount = 0;
			foreach (tk_LocationInEntry lk_LocationInEntry, lk_RightEntries)
			{
				if (ls_LeftId == lk_LocationInEntry.first)
					continue;
				++li_ComparisonCount;
				
				if (lk_Entries[lk_LocationInEntry.first].length() >= lk_Entries[ls_LeftId].length())
				{
					int li_Length = lk_Entries[ls_LeftId].length();
					if (lk_Entries[ls_LeftId].endsWith("*"))
						--li_Length;
					if (lk_Entries[ls_LeftId].left(li_Length) == lk_Entries[lk_LocationInEntry.first].mid(lk_LocationInEntry.second, li_Length))
					{
						// ls_LeftId is completely contained within the other one!
						int li_EndPos = lk_Entries[lk_LocationInEntry.first].length() - (lk_LocationInEntry.second + lk_Entries[ls_LeftId].length());
						
						printf("%5d %5d %s %s\n", lk_LocationInEntry.second, li_EndPos, ls_LeftId.toStdString().c_str(),
								lk_LocationInEntry.first.toStdString().c_str());
						++li_PairCount;
						++li_LeftIdPairCount;
						
						lk_Containing[lk_LocationInEntry.first] << ls_LeftId;
						lk_Contained[ls_LeftId] << lk_LocationInEntry.first;
						
						lk_RemainingEntries.remove(ls_LeftId);
					}
				}
			}
			if (li_LeftIdPairCount > 1)
			{
				printf("WHOOOPS! This entry is contained in several other entries (%d)!!!\n", li_LeftIdPairCount);
			}
		}
	}
	printf("entries: %d.\n", lk_Entries.size());
	printf("found %d pairs.\n", li_PairCount);
	printf("string comparisons: %d.\n", li_ComparisonCount);
	QSet<QString> lk_ContainingEntries = lk_Containing.keys().toSet();
	QSet<QString> lk_ContainedEntries = lk_Contained.keys().toSet();
	QSet<QString> lk_Chain = lk_ContainingEntries & lk_ContainedEntries;
	printf("There are %d entries that are in the middle!\n", lk_Chain.size());
	printf("entries after erasing all fakes and doubles: %d.\n", lk_RemainingEntries.size());
}
