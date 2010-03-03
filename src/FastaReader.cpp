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

#include <ptb/FastaReader.h>


k_FastaReader::k_FastaReader(QString as_Path)
    : mk_File(as_Path)
    , mk_Stream(&mk_File)
    , mb_HaveNextLine(false)
{
    if (!mk_File.open(QIODevice::ReadOnly))
    {
        printf("Error: Unable to open %s.\n", as_Path.toStdString().c_str());
        exit(1);
    }
}


k_FastaReader::~k_FastaReader()
{
    mk_File.close();
}


bool k_FastaReader::readEntry(r_FastaEntry* ar_FastaEntry_)
{
    ar_FastaEntry_->ms_Id = QString();
    ar_FastaEntry_->ms_Entry = QString();
    // fetch lines until we have an id line
    while (!this->atEnd())
    {
        this->readNextLine();
        if (ms_NextLine.startsWith(">"))
            break;
    }
    
    if (this->atEnd())
        return false;
        
    // chop off leading '>'
    ms_NextLine.remove(0, 1);
    ar_FastaEntry_->ms_Id = ms_NextLine.trimmed();
    while (!this->atEnd())
    {
        this->readNextLine();
        if (!ms_NextLine.startsWith(">"))
            ar_FastaEntry_->ms_Entry += ms_NextLine;
        else
        {
            mb_HaveNextLine = true;
            break;
        }
    }
        
    return ((!ar_FastaEntry_->ms_Id.isEmpty()) && (!ar_FastaEntry_->ms_Entry.isEmpty()));
}


void k_FastaReader::readNextLine()
{
    if (mb_HaveNextLine)
    {
        mb_HaveNextLine = false;
        return;
    }
    ms_NextLine = mk_Stream.readLine().trimmed();
}


bool k_FastaReader::atEnd()
{
    return mk_Stream.atEnd() && (!mb_HaveNextLine);
}
