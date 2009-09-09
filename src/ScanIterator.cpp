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

#include <ptb/ScanIterator.h>
#include <ptb/MzDataHandler.h>
#include <ptb/MzMlHandler.h>
#include <ptb/MzXmlHandler.h>
#include <ptb/XmlHandler.h>
#include <ptb/ZipFileOrNot.h>


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


QList<r_Peak> k_ScanIterator::findAllPeaks(r_Spectrum& ar_Spectrum)
{
	QList<r_Peak> lk_Results;
	double ld_LastIntensity = ar_Spectrum.md_IntensityValues_[0];
	int li_LastDirection = -100;
	int li_PeakIndex = -1;
	int li_ValleyIndex = -1;
	for (int i = 1; i < ar_Spectrum.mi_PeaksCount; ++i)
	{
		double ld_ThisIntensity = ar_Spectrum.md_IntensityValues_[i];
		double ld_Slope = ld_ThisIntensity - ld_LastIntensity;
		int li_Direction = 0;
		if (ld_Slope > 0)
			li_Direction = 1;
		else if (ld_Slope < 0)
			li_Direction = -1;
			
		if (li_LastDirection >= -1)
		{
			if (li_Direction == -1 && li_LastDirection != -1)
			{
				// from ascend or equal to descend (peak!)
				li_PeakIndex = i - 1;
			}
			if (li_Direction != -1 && li_LastDirection == -1)
			{
				// end of peak
				if (li_PeakIndex >= 0 && li_ValleyIndex >= 0)
				{
					// we now have a good peak
					r_Peak lr_Peak;
					lr_Peak.mi_PeakIndex = li_PeakIndex;
					lr_Peak.mi_LeftBorderIndex = li_ValleyIndex;
					lr_Peak.mi_RightBorderIndex = i - 1;
					lr_Peak.md_OutsideBorderMaxIntensity = 1e-15;
					if (lr_Peak.mi_LeftBorderIndex > 0)
						lr_Peak.md_OutsideBorderMaxIntensity = std::max<double>(lr_Peak.md_OutsideBorderMaxIntensity, ar_Spectrum.md_IntensityValues_[lr_Peak.mi_LeftBorderIndex - 1]);
					if (lr_Peak.mi_RightBorderIndex < ar_Spectrum.mi_PeaksCount - 1)
						lr_Peak.md_OutsideBorderMaxIntensity = std::max<double>(lr_Peak.md_OutsideBorderMaxIntensity, ar_Spectrum.md_IntensityValues_[lr_Peak.mi_RightBorderIndex + 1]);
					lr_Peak.md_PeakIntensity = ar_Spectrum.md_IntensityValues_[lr_Peak.mi_PeakIndex];
					// Attention, this is only the raw intensity from the scan,
					// it will be replaced by the Gauss intensity a few lines below
					lr_Peak.md_Snr = lr_Peak.md_PeakIntensity / lr_Peak.md_OutsideBorderMaxIntensity;
					
					// cut SNR at 10000 max
					// :UGLY: should not be a magic number
					if (lr_Peak.md_OutsideBorderMaxIntensity * 10000.0 < lr_Peak.md_PeakIntensity)
						lr_Peak.md_Snr = 10000.0;
					
					fitGaussian(&lr_Peak.md_GaussA, &lr_Peak.md_GaussB, &lr_Peak.md_GaussC,
								ar_Spectrum.md_MzValues_[lr_Peak.mi_PeakIndex - 1],
								ar_Spectrum.md_IntensityValues_[lr_Peak.mi_PeakIndex - 1],
								ar_Spectrum.md_MzValues_[lr_Peak.mi_PeakIndex],
								ar_Spectrum.md_IntensityValues_[lr_Peak.mi_PeakIndex],
								ar_Spectrum.md_MzValues_[lr_Peak.mi_PeakIndex + 1],
								ar_Spectrum.md_IntensityValues_[lr_Peak.mi_PeakIndex + 1]);
					lr_Peak.md_PeakMz = lr_Peak.md_GaussB;
					lr_Peak.md_PeakIntensity = lr_Peak.md_GaussA;
					lr_Peak.md_PeakArea = lr_Peak.md_GaussA * lr_Peak.md_GaussC;
					lk_Results.push_back(lr_Peak);
				}
				li_ValleyIndex = i - 1;
			}
			if (li_Direction == 1 && li_LastDirection != 1)
			{
				// start of peak
				li_ValleyIndex = i - 1;
			}
		}
		li_LastDirection = li_Direction;
		ld_LastIntensity = ld_ThisIntensity;
	}
	
	return lk_Results;
}


void k_ScanIterator::fitGaussian(double* a_, double* b_, double* c_, 
								  double x0, double y0, 
								  double x1, double y1, 
								  double x2, double y2)
{
	double A = 2.0 * (x1 - x0);
	double B = x0 * x0 - x1 * x1;
	double C = 2.0 * (x2 - x0);
	double D = x0 * x0 - x2 * x2;
	double lny0 = log(y0);
	double lny1 = log(y1);
	double lny2 = log(y2);
	double F = lny1 - lny0;
	double G = lny2 - lny0;
	double b = (F * D - B * G) / (A * G - F * C);
	double x0b = x0 - b;
	double x1b = x1 - b;
	double d = ((x0b * x0b) - (x1b * x1b)) / (lny1 - lny0);
	double a = y0 / (exp(-((x0b * x0b) / d)));
	double c = sqrt(d * 0.5);
	*a_ = a;
	*b_ = b;
	*c_ = c;
}
							   

void k_ScanIterator::progressFunction(QString, bool)
{
}

