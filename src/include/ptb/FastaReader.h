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


struct r_FastaEntry
{
	r_FastaEntry()
		: ms_Id(QString())
		, ms_Entry(QString())
	{
	}
	
	r_FastaEntry(const r_FastaEntry& ar_Other)
		: ms_Id(ar_Other.ms_Id)
		, ms_Entry(ar_Other.ms_Entry)
	{
	}
	
	QString ms_Id;
	QString ms_Entry;
};


class k_FastaReader
{
public:
	k_FastaReader(QString as_Path);
	virtual ~k_FastaReader();
	
	virtual bool readEntry(r_FastaEntry* ar_FastaEntry_);

protected:
	virtual void readNextLine();
	virtual bool atEnd();
	
	QFile mk_File;
	QTextStream mk_Stream;
	QString ms_NextLine;
	bool mb_HaveNextLine;
};
