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
#include "XmlHandler.h"


class k_MzXmlHandler: public k_XmlHandler
{
public:
	k_MzXmlHandler(k_ScanIterator& ak_ScanIterator);
	virtual ~k_MzXmlHandler();
	
	virtual bool startElement(const QString &namespaceURI, const QString &localName,
							  const QString &qName, const QXmlAttributes &attributes);

protected:
	virtual void handleElement(const QString& as_Tag, const tk_XmlAttributes& ak_Attributes, const QString as_Text);
	
	QList<r_Scan> mk_ScanStack;
};
