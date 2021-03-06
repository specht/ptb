/*
Copyright (c) 2007-2008 Michael Specht

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

#include <ptb/ZipFileOrNot.h>
#include "kfilterbase/kfilterdev.h"


k_ZipFileOrNot::k_ZipFileOrNot(QString as_Path)
    : ms_Path(as_Path)
{
    if (ms_Path.toLower().endsWith(".zip"))
    {
        mk_pZip = QSharedPointer<QuaZip>(new QuaZip(ms_Path));
        mk_pZip->open(QuaZip::mdUnzip);
        mk_pDevice = QSharedPointer<QIODevice>(new QuaZipFile(mk_pZip.data()));
        mk_pZip->goToFirstFile();
    } 
    else if (ms_Path.toLower().endsWith(".bz2"))
    {
        mk_pDevice = QSharedPointer<QIODevice>(KFilterDev::deviceForFile(as_Path));
    }
    else if (ms_Path.toLower().endsWith(".gz"))
    {
        mk_pDevice = QSharedPointer<QIODevice>(KFilterDev::deviceForFile(as_Path));
    }
    else
        mk_pDevice = QSharedPointer<QIODevice>(new QFile(as_Path));
}


k_ZipFileOrNot::~k_ZipFileOrNot()
{
}


QIODevice* k_ZipFileOrNot::device()
{
    return mk_pDevice.data();
}
