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

#include "MgfWriter.h"
#include <QtCore>


k_MgfWriter::k_MgfWriter(r_ScanType::Enumeration ae_ScanType,
						 QList<tk_IntPair> ak_MsLevels)
	: k_ScanIterator(ae_ScanType, ak_MsLevels)
	, mb_UseInputCharges(true)
	, mk_RetentionTimesFile_(NULL)
	, mk_RetentionTimesStream_(NULL)
{
	mk_TextStream.setRealNumberNotation(QTextStream::FixedNotation);
	mk_TextStream.setRealNumberPrecision(10);
	mk_TextStream.setNumberFlags(QTextStream::ForcePoint);
}


k_MgfWriter::~k_MgfWriter()
{
}


void k_MgfWriter::convert(QStringList ak_SpectraFiles, QString as_OutputPath, int ai_BatchSize, QString as_RetentionTimesPath)
{
	ms_OutputPath = as_OutputPath;
	mi_BatchSize = ai_BatchSize;
	mi_PartCounter = 0;
	mi_CurrentBatchSize = 0;
	if (!as_RetentionTimesPath.isEmpty())
	{
		mk_RetentionTimesFile_ = new QFile(as_RetentionTimesPath);
		if (!mk_RetentionTimesFile_->open(QIODevice::WriteOnly))
		{
			printf("Error: Unable to open %s for writing.\n", as_RetentionTimesPath.toStdString().c_str());
			exit(1);
		}
		mk_RetentionTimesStream_ = new QTextStream(mk_RetentionTimesFile_);
	}
	foreach (QString ls_Path, ak_SpectraFiles)
	{
		ms_SpotName = QFileInfo(ls_Path).baseName();
		this->parseFile(ls_Path);
	}
	if (mk_RetentionTimesFile_)
	{
		mk_RetentionTimesStream_->flush();
		delete mk_RetentionTimesStream_;
		mk_RetentionTimesStream_ = NULL;
		mk_RetentionTimesFile_->close();
		delete mk_RetentionTimesFile_;
		mk_RetentionTimesFile_ = NULL;
	}
}


void k_MgfWriter::handleScan(r_Scan& ar_Scan)
{
	if (ar_Scan.mr_Spectrum.mi_PeaksCount == 0)
	{
		printf("Warning: Empty spectrum (scan #%s @ %1.2f minutes)!\n", ar_Scan.ms_Id.toStdString().c_str(), ar_Scan.md_RetentionTime);
		return;
	}
	if (!ar_Scan.mk_Precursors.isEmpty())
	{
		foreach (r_Precursor lr_Precursor, ar_Scan.mk_Precursors)
			this->flushScan(ar_Scan, &lr_Precursor);
	}
	else
		this->flushScan(ar_Scan);
}


void k_MgfWriter::flushScan(r_Scan& ar_Scan, r_Precursor* ar_Precursor_)
{
	if (!mk_TextStream.device() || ((mi_BatchSize > 0) && (mi_CurrentBatchSize >= mi_BatchSize)))
	{
		QString ls_Filename = ms_OutputPath;
		if (mi_BatchSize > 0)
		{
			ls_Filename += QString(".%1").arg(mi_PartCounter);
			++mi_PartCounter;
			mi_CurrentBatchSize = 0;
		}
		mk_pFile = RefPtr<QFile>(new QFile(ls_Filename));
		if (!mk_pFile->open(QIODevice::WriteOnly))
		{
			printf("Error: Unable to open %s for writing.\n", ls_Filename.toStdString().c_str());
			exit(1);
		}
		mk_TextStream.setDevice(mk_pFile.get_Pointer());
	}
	if (mk_RetentionTimesStream_)
		*mk_RetentionTimesStream_ << ms_SpotName << "." << ar_Scan.ms_Id << ": " << ar_Scan.md_RetentionTime << "\n";
	if (!ar_Precursor_)
	{
		mk_TextStream << "BEGIN IONS\n";
		mk_TextStream << "TITLE=" << ms_SpotName << "." << ar_Scan.ms_Id << "." << ar_Scan.ms_Id << ".x\n";
		for (int i = 0; i < ar_Scan.mr_Spectrum.mi_PeaksCount; ++i)
			mk_TextStream << this->ftos(ar_Scan.mr_Spectrum.md_MzValues_[i]) << " " << this->ftos(ar_Scan.mr_Spectrum.md_IntensityValues_[i]) << "\n";
		mk_TextStream << "END IONS\n\n";
	}
	else
	{
		mk_TextStream << "BEGIN IONS\n";
		mk_TextStream << "TITLE=" << ms_SpotName << "." << ar_Scan.ms_Id << "." << ar_Scan.ms_Id << "." << ar_Precursor_->mi_ChargeState << "\n";
		mk_TextStream << "PEPMASS=" << this->ftos(ar_Precursor_->md_Mz) << "\n";
		mk_TextStream << "CHARGE=" << ar_Precursor_->mi_ChargeState << "\n";
		for (int i = 0; i < ar_Scan.mr_Spectrum.mi_PeaksCount; ++i)
			mk_TextStream << this->ftos(ar_Scan.mr_Spectrum.md_MzValues_[i]) << " " << this->ftos(ar_Scan.mr_Spectrum.md_IntensityValues_[i]) << "\n";
		mk_TextStream << "END IONS\n\n";
	}
	++mi_CurrentBatchSize;
}


QString k_MgfWriter::ftos(double ad_Value)
{
	char lc_Temp_[1024];
	sprintf(lc_Temp_, "%1.6lf", ad_Value);
	QString ls_Result(lc_Temp_);
	if (ls_Result.contains("."))
	{
		while (ls_Result.endsWith("0"))
			ls_Result.truncate(ls_Result.length() - 1);
		if (ls_Result.endsWith("."))
			ls_Result.truncate(ls_Result.length() - 1);
	}
	return ls_Result;
}
