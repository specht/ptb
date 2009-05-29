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

#include "ScanIterator.h"
#include "MzDataHandler.h"
#include "MzMlHandler.h"
#include "MzXmlHandler.h"
#include "XmlHandler.h"
#include "ZipFileOrNot.h"


k_ScanIterator::k_ScanIterator(r_ScanType::Enumeration ae_ScanType,
							   QList<tk_IntPair> ak_ScanMsLevels)
	: me_ScanType(ae_ScanType)
	, mk_ScanMsLevels(ak_ScanMsLevels)
{
}


k_ScanIterator::~k_ScanIterator()
{
}


void k_ScanIterator::parseFile(QString as_Filename)
{
	k_ZipFileOrNot lk_ZipFileOrNot(as_Filename);
	
	if (!lk_ZipFileOrNot.device()->open(QIODevice::ReadOnly))
	{
		printf("Error: Unable to open %s.\n", as_Filename.toStdString().c_str());
		return;
	}
	
	RefPtr<k_XmlHandler> lk_pHandler;
	
	QString ls_Line;
	QTextStream lk_Stream(lk_ZipFileOrNot.device());
	while (ls_Line.trimmed().startsWith("<?") || ls_Line.trimmed().isEmpty())
		ls_Line = lk_Stream.readLine();
	if (ls_Line.trimmed().startsWith("<mzXML"))
		lk_pHandler = RefPtr<k_XmlHandler>(new k_MzXmlHandler(*this));
	else if (ls_Line.trimmed().startsWith("<mzData"))
		lk_pHandler = RefPtr<k_XmlHandler>(new k_MzDataHandler(*this));
	else if (ls_Line.trimmed().startsWith("<mzML") || ls_Line.trimmed().startsWith("<indexedmzML"))
		lk_pHandler = RefPtr<k_XmlHandler>(new k_MzMlHandler(*this));
	else
	{
		printf("Error: Unable to parse input file %s.\n", as_Filename.toStdString().c_str());
		return;
	}
	
	// re-open the file, reset() didn't work in some cases, but why?!
	lk_ZipFileOrNot = k_ZipFileOrNot(as_Filename);
	lk_ZipFileOrNot.device()->open(QIODevice::ReadOnly);
    
    QXmlSimpleReader lk_Reader;
    lk_Reader.setContentHandler(lk_pHandler.get_Pointer());
    
    QXmlInputSource lk_InputSource(lk_ZipFileOrNot.device());
    lk_Reader.parse(&lk_InputSource);
}


bool k_ScanIterator::isInterestingScan(r_Scan& ar_Scan)
{
	// check MS level
	bool lb_GoodMsLevel = false;
	foreach (tk_IntPair lk_IntPair, mk_ScanMsLevels)
	{
		if (ar_Scan.mi_MsLevel >= lk_IntPair.first && ar_Scan.mi_MsLevel <= lk_IntPair.second)
		{
			lb_GoodMsLevel = true;
			break;
		}
	}
	if (!lb_GoodMsLevel)
		return false;
		
	// if we made it until here, check whether scan type is interesting
	return (ar_Scan.me_Type & me_ScanType) != 0;
}


void k_ScanIterator::convertValues(QByteArray ak_Data, int ai_Size, int ai_Precision, 
								   bool ab_NetworkByteOrder, QList<double*>& ak_Targets)
{
	for (int i = 0; i < ak_Targets.size(); ++i)
		ak_Targets[i] = NULL;
		
	if (ai_Size <= 0)
		return;
		
	if (!(ai_Precision == 32 || ai_Precision == 64))
	{
		printf("Error: Invalid float precision: %d. (must be 32 or 64)\n", ai_Precision);
		// do you think it's a bit exaggerated here to crash the program?
		// precision is a touchy subject, after all, isn't it?
		exit(1);
	}
		
	for (int i = 0; i < ak_Targets.size(); ++i)
	{
		ak_Targets[i] = new double[ai_Size];
		if (!ak_Targets[i])
		{
			printf("Error: Out of memory.\n");
			exit(1);
		}
	}
		
	const unsigned char* luc_Buffer_ = (const unsigned char*)ak_Data.constData();
	bool lb_SystemHasNetworkByteOrder = QSysInfo::ByteOrder == QSysInfo::BigEndian;
	bool lb_NeedSwap = lb_SystemHasNetworkByteOrder ^ ab_NetworkByteOrder;
	
	QList<double*> lk_Targets = ak_Targets;
	
	for (int li_Offset = 0; li_Offset < ai_Size; ++li_Offset)
	{
		for (int li_Target = 0; li_Target < ak_Targets.size(); ++li_Target)
		{
			double ld_NextValue;
			
			if (ai_Precision == 32)
			{
				float lf_Value;
				memcpy(&lf_Value, luc_Buffer_, 4);
				luc_Buffer_ += 4;
				if (lb_NeedSwap)
				{
					float lf_Temp = lf_Value;
					*((unsigned char*)(&lf_Value) + 0) = *((unsigned char*)(&lf_Temp) + 3);
					*((unsigned char*)(&lf_Value) + 1) = *((unsigned char*)(&lf_Temp) + 2);
					*((unsigned char*)(&lf_Value) + 2) = *((unsigned char*)(&lf_Temp) + 1);
					*((unsigned char*)(&lf_Value) + 3) = *((unsigned char*)(&lf_Temp) + 0);
				}
				ld_NextValue = (double)lf_Value;
			}
			else if (ai_Precision == 64)
			{
				memcpy(&ld_NextValue, luc_Buffer_, 8);
				luc_Buffer_ += 8;
				if (lb_NeedSwap)
				{
					double ld_Temp = ld_NextValue;
					*((unsigned char*)(&ld_NextValue) + 0) = *((unsigned char*)(&ld_Temp) + 7);
					*((unsigned char*)(&ld_NextValue) + 1) = *((unsigned char*)(&ld_Temp) + 6);
					*((unsigned char*)(&ld_NextValue) + 2) = *((unsigned char*)(&ld_Temp) + 5);
					*((unsigned char*)(&ld_NextValue) + 3) = *((unsigned char*)(&ld_Temp) + 4);
					*((unsigned char*)(&ld_NextValue) + 4) = *((unsigned char*)(&ld_Temp) + 3);
					*((unsigned char*)(&ld_NextValue) + 5) = *((unsigned char*)(&ld_Temp) + 2);
					*((unsigned char*)(&ld_NextValue) + 6) = *((unsigned char*)(&ld_Temp) + 1);
					*((unsigned char*)(&ld_NextValue) + 7) = *((unsigned char*)(&ld_Temp) + 0);
				}
			}
			
			*(lk_Targets[li_Target]) = ld_NextValue;
			++(lk_Targets[li_Target]);
		}
	}
}


void k_ScanIterator::progressFunction(QString, bool)
{
}

