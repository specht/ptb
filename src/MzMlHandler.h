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
#include "RefPtr.h"


class k_MzMlHandler: public k_XmlHandler
{
public:
	k_MzMlHandler(k_ScanIterator& ak_ScanIterator);
	virtual ~k_MzMlHandler();
	
	virtual bool startElement(const QString &namespaceURI, const QString &localName,
							  const QString &qName, const QXmlAttributes &attributes);

protected:
	virtual void handleElement(const QString& as_Tag, const tk_XmlAttributes& ak_Attributes, const QString as_Text);
	
	RefPtr<r_Scan> mr_pCurrentScan;
	QString ms_BinaryPrecision;
	QString ms_BinaryCompression;
	QString ms_BinaryType;
	QString ms_PrecursorMz;
	QString ms_PrecursorIntensity;
	QString ms_PrecursorChargeState;
};
