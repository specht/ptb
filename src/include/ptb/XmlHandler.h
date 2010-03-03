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
#include <QtXml>
#include <ptb/ScanIterator.h>


typedef QHash<QString, QString> tk_XmlAttributes;
typedef QPair<QString, tk_XmlAttributes> tk_XmlElement;


class k_XmlHandler: public QXmlDefaultHandler
{
public:
    k_XmlHandler(k_ScanIterator& ak_ScanIterator);
    virtual ~k_XmlHandler();

    virtual bool startElement(const QString &namespaceURI, const QString &localName,
                              const QString &qName, const QXmlAttributes &attributes);
    virtual bool endElement(const QString &namespaceURI, const QString &localName,
                            const QString &qName);
    virtual bool characters(const QString &str);
    
protected:
    virtual void handleElement(const QString& as_Tag, const tk_XmlAttributes& ak_Attributes, const QString as_Text) = 0;
    
    k_ScanIterator& mk_ScanIterator;
    QList<tk_XmlElement> mk_XmlPath;
    QList<QString> mk_XmlPathText;
};
