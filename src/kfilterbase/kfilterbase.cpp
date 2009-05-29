/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfilterbase.h"
#include "kde_config.h"

//#include <kdebug.h>
#include <QtCore/QIODevice>
//#include <kmimetype.h>
#include "kgzipfilter.h"
#ifdef HAVE_BZIP2_SUPPORT
#include "kbzip2filter.h"
#endif

KFilterBase::KFilterBase()
    : m_dev( 0L ), m_bAutoDel( false ), d(0)
{
}

KFilterBase::~KFilterBase()
{
    if ( m_bAutoDel )
        delete m_dev;
}

void KFilterBase::setDevice( QIODevice * dev, bool autodelete )
{
    m_dev = dev;
    m_bAutoDel = autodelete;
}

QIODevice * KFilterBase::device()
{
    return m_dev;
}

bool KFilterBase::inBufferEmpty() const
{
    return inBufferAvailable() == 0;
}

bool KFilterBase::outBufferFull() const
{
    return outBufferAvailable() == 0;
}

KFilterBase * KFilterBase::findFilterByFileName( const QString & fileName )
{
    if ( fileName.endsWith( ".gz", Qt::CaseInsensitive ) )
    {
        return new KGzipFilter;
    }
#ifdef HAVE_BZIP2_SUPPORT
    if ( fileName.endsWith( ".bz2", Qt::CaseInsensitive ) )
    {
        return new KBzip2Filter;
    }
#endif

    return 0;
}

void KFilterBase::terminate()
{
}

void KFilterBase::reset()
{
}

void KFilterBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

