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

#include "XmlHandler.h"


k_XmlHandler::k_XmlHandler(k_ScanIterator& ak_ScanIterator)
	: mk_ScanIterator(ak_ScanIterator)
{
}


k_XmlHandler::~k_XmlHandler()
{
}


bool k_XmlHandler::startElement(const QString &namespaceURI, const QString &localName,
								const QString &qName, const QXmlAttributes &attributes)
{
	tk_XmlAttributes lk_Attributes;
	for (int i = 0; i < attributes.count(); ++i)
		lk_Attributes[attributes.qName(i)] = attributes.value(i);
	
	tk_XmlElement lk_Element(qName, lk_Attributes);
	mk_XmlPath.push_front(lk_Element);
	mk_XmlPathText.push_front(QString());
	return true;
}


bool k_XmlHandler::endElement(const QString &namespaceURI, const QString &localName,
							  const QString &qName)
{
	this->handleElement(mk_XmlPath.front().first, mk_XmlPath.front().second,
						mk_XmlPathText.front());
	mk_XmlPathText.pop_front();
	mk_XmlPath.pop_front();
	return true;
}


bool k_XmlHandler::characters(const QString &str)
{
	mk_XmlPathText.first() += str;
	return true;
}
