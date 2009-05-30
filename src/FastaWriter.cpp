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

#include <ptb/FastaWriter.h>


k_FastaWriter::k_FastaWriter(QIODevice* ak_Device_)
	: mk_Device_(ak_Device_)
	, mk_Stream(mk_Device_)
{
}


k_FastaWriter::~k_FastaWriter()
{
	mk_Stream.flush();
}


void k_FastaWriter::writeEntry(r_FastaEntry& ar_FastaEntry)
{
	mk_Stream << ">" << ar_FastaEntry.ms_Id << "\n";
	int li_Index = 0;
	while (li_Index < ar_FastaEntry.ms_Entry.length())
	{
		mk_Stream << ar_FastaEntry.ms_Entry.mid(li_Index, 70) << "\n";
		li_Index += 70;
	}
}
