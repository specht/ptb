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


typedef QHash<QString, QString> tk_XmlAttributes;
typedef QPair<QString, tk_XmlAttributes> tk_XmlElement;
typedef QPair<int, int> tk_IntPair;


struct r_ScanType
{
	enum Enumeration
	{
		Unknown = 0,
		MS1 = 1,
		MSn = 2,
		CRM = 4,
		SIM = 8,
		SRM = 16,
		PDA = 32,
		SICC = 64,
		All = MS1 | MSn | CRM | SIM | SRM | PDA | SICC
	};
};

	
struct r_Spectrum
{
	r_Spectrum()
		: mi_PeaksCount(0)
		, md_MzValues_(NULL)
		, md_IntensityValues_(NULL)
	{
	};
	
	r_Spectrum(const r_Spectrum& ar_Other)
		: mi_PeaksCount(ar_Other.mi_PeaksCount)
		, md_MzValues_(NULL)
		, md_IntensityValues_(NULL)
	{
		// copy m/z and intensity values, if any
		if (ar_Other.md_MzValues_)
		{
			md_MzValues_ = new double[mi_PeaksCount];
			memcpy(md_MzValues_, ar_Other.md_MzValues_, mi_PeaksCount * sizeof(double));
		}
		if (ar_Other.md_IntensityValues_)
		{
			md_IntensityValues_ = new double[mi_PeaksCount];
			memcpy(md_IntensityValues_, ar_Other.md_IntensityValues_, mi_PeaksCount * sizeof(double));
		}
	};
	
	virtual ~r_Spectrum()
	{
		if (md_MzValues_)
		{
			delete [] md_MzValues_;
			md_MzValues_ = NULL;
		}
		if (md_IntensityValues_)
		{
			delete [] md_IntensityValues_;
			md_IntensityValues_ = NULL;
		}
	}
	
	int mi_PeaksCount;
	double* md_MzValues_;
	double* md_IntensityValues_;
};


struct r_Precursor
{
	r_Precursor()
		: md_Mz(0.0)
		, md_Intensity(0.0)
		, mi_ChargeState(0)
	{
	};
	
	r_Precursor(double ad_Mz, double ad_Intensity, int ai_ChargeState)
		: md_Mz(ad_Mz)
		, md_Intensity(ad_Intensity)
		, mi_ChargeState(ai_ChargeState)
	{
	};
	
	double md_Mz;
	double md_Intensity;
	int mi_ChargeState;
};


struct r_Scan
{
	r_Scan()
		: ms_Id(QString())
		, md_RetentionTime(0.0)
		, me_Type(r_ScanType::Unknown)
		, mi_MsLevel(0)
		, ms_FilterLine(QString())
		, mr_Spectrum(r_Spectrum())
		, mk_Precursors(QList<r_Precursor>())
	{
	}
	
	r_Scan(const r_Scan& ar_Other)
		: ms_Id(ar_Other.ms_Id)
		, md_RetentionTime(ar_Other.md_RetentionTime)
		, me_Type(ar_Other.me_Type)
		, mi_MsLevel(ar_Other.mi_MsLevel)
		, ms_FilterLine(ar_Other.ms_FilterLine)
		, mr_Spectrum(ar_Other.mr_Spectrum)
		, mk_Precursors(ar_Other.mk_Precursors)
	{
	}
	
	QString ms_Id;
	double md_RetentionTime;
	r_ScanType::Enumeration me_Type;
	int mi_MsLevel;
	QString ms_FilterLine;
	r_Spectrum mr_Spectrum;
	QList<r_Precursor> mk_Precursors;
};


struct r_Peak
{
	int mi_PeakIndex;
	int mi_LeftBorderIndex;
	int mi_RightBorderIndex;
	// attention, these are gauss peak m/z and intensity values! yay!
	double md_PeakMz;
	double md_PeakIntensity;
	double md_PeakArea;
	double md_OutsideBorderMaxIntensity;
	double md_Snr;
	double md_GaussA;
	double md_GaussB;
	double md_GaussC;
};


class k_ScanIterator
{
public:
	k_ScanIterator(r_ScanType::Enumeration ae_ScanType = r_ScanType::All,
				   QList<tk_IntPair> ak_MsLevels = QList<tk_IntPair>() << tk_IntPair(1, 0x10000));
	virtual ~k_ScanIterator();
	
	virtual void parseFile(QString as_Filename);
	
	static void convertValues(QByteArray ak_Data, int ai_Size, int ai_Precision, 
							  bool ab_NetworkByteOrder, QList<double*>& ak_Targets);
	static QList<r_Peak> findAllPeaks(r_Spectrum& ar_Spectrum);
	static void fitGaussian(double* a_, double* b_, double* c_, 
							 double x0, double y0, 
							 double x1, double y1, 
							 double x2, double y2);
	
	virtual bool isInterestingScan(r_Scan& ar_Scan);
	virtual void handleScan(r_Scan& ar_Scan) = 0;
	virtual void progressFunction(QString as_ScanId, bool ab_InterestingScan);

protected:
	
	r_ScanType::Enumeration me_ScanType;
	QList<tk_IntPair> mk_ScanMsLevels;
};
