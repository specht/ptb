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

#include <ptb/MzMlHandler.h>
#include <ptb/ScanIterator.h>

#define CV_MS_LEVEL "MS:1000511"
#define CV_MS_SCAN_TYPE_MS1 "MS:1000579"
#define CV_MS_SCAN_TYPE_MSn "MS:1000580"
#define CV_MS_SCAN_TYPE_CRM "MS:1000581"
#define CV_MS_SCAN_TYPE_SIM "MS:1000582"
#define CV_MS_SCAN_TYPE_SRM "MS:1000583"
#define CV_MS_SCAN_TYPE_PDA "MS:1000620"
#define CV_MS_SCAN_TYPE_SICC "MS:1000627"
#define CV_MS_FILTER_STRING "MS:1000512"
#define CV_MS_RETENTION_TIME "MS:1000016"
#define CV_MS_TIME_UNIT_MINUTE "MS:1000038"
#define CV_MS_TIME_UNIT_SECOND "MS:1000039"
#define CV_MS_32_BIT_FLOAT "MS:1000521"
#define CV_MS_64_BIT_FLOAT "MS:1000523"
#define CV_MS_NO_COMPRESSION "MS:1000576"
#define CV_MS_ZLIB_COMPRESSION "MS:1000574"
#define CV_MS_MZ_ARRAY "MS:1000514"
#define CV_MS_INTENSITY_ARRAY "MS:1000515"
#define CV_MS_MZ "MS:1000040"
#define CV_MS_CHARGE_STATE "MS:1000041"
#define CV_MS_INTENSITY "MS:1000042"
#define CV_MS_SPECTRUM_REPRESENTATION_CENTROID "MS:1000127"
#define CV_MS_SPECTRUM_REPRESENTATION_PROFILE "MS:1000128"


k_MzMlHandler::k_MzMlHandler(k_ScanIterator& ak_ScanIterator)
    : k_XmlHandler(ak_ScanIterator)
    , mr_pCurrentScan(new r_Scan())
{
}


k_MzMlHandler::~k_MzMlHandler()
{
}


bool k_MzMlHandler::startElement(const QString &namespaceURI, const QString &localName,
                                 const QString &qName, const QXmlAttributes &attributes)
{
    if (mb_Cancelled)
        return false;
    
    if (qName == "binaryDataArray")
    {
        ms_BinaryPrecision = "";
        ms_BinaryCompression = "";
        ms_BinaryType = "";
    }
    else if (qName == "selectedIon")
    {
        ms_PrecursorMz = "";
        ms_PrecursorIntensity = "0.0";
        ms_PrecursorChargeState = "";   
    }
    else if (qName == "spectrum")
    {
        // create a fresh scan
        mr_pCurrentScan = QSharedPointer<r_Scan>(new r_Scan());
    }
        
    return k_XmlHandler::startElement(namespaceURI, localName, qName, attributes);
}


void k_MzMlHandler::handleElement(const QString& as_Tag, const tk_XmlAttributes& ak_Attributes, const QString as_Text)
{
    if (as_Tag == "cvParam")
    {
        QString ls_Accession = ak_Attributes["accession"];
        if (ls_Accession == CV_MS_LEVEL)
            mr_pCurrentScan->mi_MsLevel = QVariant(ak_Attributes["value"]).toInt();
        else if (ls_Accession == CV_MS_SCAN_TYPE_MS1)
            mr_pCurrentScan->me_Type = r_ScanType::MS1;
        else if (ls_Accession == CV_MS_SCAN_TYPE_MSn)
            mr_pCurrentScan->me_Type = r_ScanType::MSn;
        else if (ls_Accession == CV_MS_SCAN_TYPE_CRM)
            mr_pCurrentScan->me_Type = r_ScanType::CRM;
        else if (ls_Accession == CV_MS_SCAN_TYPE_SIM)
            mr_pCurrentScan->me_Type = r_ScanType::SIM;
        else if (ls_Accession == CV_MS_SCAN_TYPE_SRM)
            mr_pCurrentScan->me_Type = r_ScanType::SRM;
        else if (ls_Accession == CV_MS_SCAN_TYPE_PDA)
            mr_pCurrentScan->me_Type = r_ScanType::PDA;
        else if (ls_Accession == CV_MS_SCAN_TYPE_SICC)
            mr_pCurrentScan->me_Type = r_ScanType::SICC;
        else if (ls_Accession == CV_MS_FILTER_STRING)
            mr_pCurrentScan->ms_FilterLine = ak_Attributes["value"];
        else if (ls_Accession == CV_MS_SPECTRUM_REPRESENTATION_CENTROID)
            mr_pCurrentScan->mr_Spectrum.mb_Centroided = true;
        else if (ls_Accession == CV_MS_SPECTRUM_REPRESENTATION_PROFILE)
            mr_pCurrentScan->mr_Spectrum.mb_Centroided = false;
        else if (ls_Accession == CV_MS_RETENTION_TIME)
        {
            mr_pCurrentScan->md_RetentionTime = QVariant(ak_Attributes["value"]).toDouble();
            if (ak_Attributes.contains("unitAccession"))
                if (ak_Attributes["unitAccession"] == CV_MS_TIME_UNIT_SECOND)
                    mr_pCurrentScan->md_RetentionTime /= 60.0;
        }
        else if (ls_Accession == CV_MS_32_BIT_FLOAT || ls_Accession == CV_MS_64_BIT_FLOAT)
            ms_BinaryPrecision = ls_Accession;
        else if (ls_Accession == CV_MS_NO_COMPRESSION || ls_Accession == CV_MS_ZLIB_COMPRESSION)
            ms_BinaryCompression = ls_Accession;
        else if (ls_Accession == CV_MS_MZ_ARRAY || ls_Accession == CV_MS_INTENSITY_ARRAY)
            ms_BinaryType = ls_Accession;
        else if (ls_Accession == CV_MS_MZ)
        {
            if (mk_XmlPath[1].first == "selectedIon")
                ms_PrecursorMz = ak_Attributes["value"];
        }
        else if (ls_Accession == CV_MS_INTENSITY)
        {
            if (mk_XmlPath[1].first == "selectedIon")
                ms_PrecursorIntensity = ak_Attributes["value"];
        }
        else if (ls_Accession == CV_MS_CHARGE_STATE)
        {
            if (mk_XmlPath[1].first == "selectedIon")
                ms_PrecursorChargeState = ak_Attributes["value"];
        }
    } 
    else if (as_Tag == "binary")
    {
        mr_pCurrentScan->mr_Spectrum.mi_PeaksCount = QVariant(mk_XmlPath[3].second["defaultArrayLength"]).toInt();
        if (ms_BinaryPrecision.isEmpty() || ms_BinaryCompression.isEmpty() || ms_BinaryType.isEmpty())
        {
            // skip this scan, maybe it's a TIC scan or something like that... eew - TIC!
            return;
            printf("Warning: missing attributes for binary array of scan %s (got precision: %s, compression: %s, type: %s).\n", 
                mk_XmlPath[3].second["nativeID"].toStdString().c_str(),
                ms_BinaryPrecision.toStdString().c_str(),
                ms_BinaryCompression.toStdString().c_str(),
                ms_BinaryType.toStdString().c_str());
            exit(1);
        }
        double* ld_Target_;
        QList<double*> lk_Targets;
        lk_Targets << ld_Target_;
            
        QByteArray lk_Bytes;
        if (ms_BinaryCompression == CV_MS_NO_COMPRESSION)
            lk_Bytes = QByteArray::fromBase64(as_Text.toAscii());
        else
        {
            lk_Bytes = QByteArray::fromBase64(as_Text.toAscii());
            quint32 lui_Length = mr_pCurrentScan->mr_Spectrum.mi_PeaksCount * (ms_BinaryPrecision == CV_MS_32_BIT_FLOAT ? 32 : 64) / 8;
            bool lb_SystemHasNetworkByteOrder = QSysInfo::ByteOrder == QSysInfo::BigEndian;
            if (!lb_SystemHasNetworkByteOrder)
            {
                quint32 lui_Temp = lui_Length;
                ((unsigned char*)&lui_Length)[0] = ((unsigned char*)&lui_Temp)[3];
                ((unsigned char*)&lui_Length)[1] = ((unsigned char*)&lui_Temp)[2];
                ((unsigned char*)&lui_Length)[2] = ((unsigned char*)&lui_Temp)[1];
                ((unsigned char*)&lui_Length)[3] = ((unsigned char*)&lui_Temp)[0];
            }
            QByteArray lk_Length((const char*)&lui_Length, 4);
            lk_Bytes.prepend(lk_Length);
            lk_Bytes = qUncompress(lk_Bytes);
            if (lk_Bytes.isEmpty())
            {
                printf("Error: the uncompressed peaks list is empty.\n");
                exit(1);
            }
        }
        k_ScanIterator::convertValues(lk_Bytes, mr_pCurrentScan->mr_Spectrum.mi_PeaksCount, 
            ms_BinaryPrecision == CV_MS_32_BIT_FLOAT ? 32 : 64, false, lk_Targets);
        
        if (ms_BinaryType == CV_MS_MZ_ARRAY)
            mr_pCurrentScan->mr_Spectrum.md_MzValues_ = lk_Targets.first();
        else
            mr_pCurrentScan->mr_Spectrum.md_IntensityValues_ = lk_Targets.first();
    }
    else if (as_Tag == "selectedIon")
    {
        mr_pCurrentScan->mk_Precursors.push_back(r_Precursor(
            QVariant(ms_PrecursorMz).toDouble(), 
            QVariant(ms_PrecursorIntensity).toDouble(), 
            QVariant(ms_PrecursorChargeState).toInt()));
    }
    else if (as_Tag == "spectrum")
    {
        if (mr_pCurrentScan->mr_Spectrum.md_MzValues_ && mr_pCurrentScan->mr_Spectrum.md_IntensityValues_)
        {
            mr_pCurrentScan->ms_Id = ak_Attributes["nativeID"];
            bool lb_Interesting = mk_ScanIterator.isInterestingScan(*(mr_pCurrentScan.data()));
            mk_ScanIterator.progressFunction(mr_pCurrentScan->ms_Id, lb_Interesting);
            if (lb_Interesting)
            {
                bool lb_Continue = true;
                mk_ScanIterator.handleScan(*(mr_pCurrentScan.data()), lb_Continue);
                if (!lb_Continue)
                    cancelParsing();
            }
        }
    }
}

