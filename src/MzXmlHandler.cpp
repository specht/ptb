/*
Copyright (c) 2007-2008 Michael Specht

This file is part of Proteomics Toolbox.

Proteomics Toolbox is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Proteomics Toolbox is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Proteomics Toolbox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ptb/MzXmlHandler.h>
#include <ptb/ScanIterator.h>


k_MzXmlHandler::k_MzXmlHandler(k_ScanIterator& ak_ScanIterator)
    : k_XmlHandler(ak_ScanIterator)
{
}


k_MzXmlHandler::~k_MzXmlHandler()
{
}


bool k_MzXmlHandler::startElement(const QString &namespaceURI, const QString &localName,
                                  const QString &qName, const QXmlAttributes &attributes)
{
    if (mb_Cancelled)
        return false;
    
    if (qName == "scan")
        mk_ScanStack.push_front(r_Scan());
        
    return k_XmlHandler::startElement(namespaceURI, localName, qName, attributes);
}


void k_MzXmlHandler::handleElement(const QString& as_Tag, const tk_XmlAttributes& ak_Attributes, const QString as_Text)
{
    if (as_Tag == "precursorMz")
    {
        double ld_PrecursorMz = QVariant(as_Text).toDouble();
        // TODO: precursor m/z is taken from filter line... is that good?
        // if there are multiple precursor m/z as in ms3 scans, the first on is chosen
        tk_XmlAttributes lk_ScanAttributes = mk_XmlPath[1].second;
        QString ls_FilterLine = lk_ScanAttributes["filterLine"];
        QRegExp lk_RegExp(".*ms[0-9]+\\s*([0-9\\.]+)@.*");
        if (lk_RegExp.indexIn(ls_FilterLine) != -1)
        {
            ld_PrecursorMz = QVariant(lk_RegExp.cap(1)).toDouble();
        }
        /*
        else
            printf("yay! %s\n", ls_FilterLine.toStdString().c_str());
            */
        double ld_PrecursorIntensity = QVariant(ak_Attributes["precursorIntensity"]).toDouble();
        int li_ChargeState = QVariant(ak_Attributes["precursorCharge"]).toInt();
        mk_ScanStack.front().mk_Precursors.push_back(r_Precursor(ld_PrecursorMz, ld_PrecursorIntensity, li_ChargeState));
    } 
    else if (as_Tag == "peaks")
    {
        tk_XmlAttributes lk_ScanAttributes = mk_XmlPath[1].second;
        mk_ScanStack.front().ms_Id = lk_ScanAttributes["num"];
        QString ls_RetentionTime = lk_ScanAttributes["retentionTime"];
        ls_RetentionTime.replace("PT", "");
        ls_RetentionTime.replace("S", "");
        mk_ScanStack.front().md_RetentionTime = QVariant(ls_RetentionTime).toDouble() / 60.0;
        if (lk_ScanAttributes["scanType"].toLower() == "full")
            mk_ScanStack.front().me_Type = r_ScanType::MSn;
        else if (lk_ScanAttributes["scanType"].toLower() == "sim")
            mk_ScanStack.front().me_Type = r_ScanType::SIM;
        else
            mk_ScanStack.front().me_Type = r_ScanType::Unknown;
        mk_ScanStack.front().mi_MsLevel = QVariant(lk_ScanAttributes["msLevel"]).toInt();
        mk_ScanStack.front().ms_FilterLine = lk_ScanAttributes["filterLine"];
        
        bool lb_InterestingScan = mk_ScanIterator.isInterestingScan(mk_ScanStack.front());
        mk_ScanIterator.progressFunction(mk_ScanStack.front().ms_Id, lb_InterestingScan);
        
        if (lb_InterestingScan)
        {
            mk_ScanStack.front().mr_Spectrum.mi_PeaksCount = QVariant(lk_ScanAttributes["peaksCount"]).toInt();
            QList<double*> lk_Targets;
            lk_Targets << mk_ScanStack.front().mr_Spectrum.md_MzValues_;
            lk_Targets << mk_ScanStack.front().mr_Spectrum.md_IntensityValues_;
            QString ls_Compression;
            if (mk_XmlPath.first().second.contains("compressionType"))
                ls_Compression = mk_XmlPath.first().second["compressionType"];
            QByteArray lk_Bytes;
            if (ls_Compression == "zlib")
            {
                // zlib compressed peaks
                int li_CompressedLength = QVariant(mk_XmlPath.first().second["compressedLen"]).toInt();
                lk_Bytes = QByteArray::fromBase64(as_Text.toAscii());
                if (lk_Bytes.size() != li_CompressedLength)
                {
                    printf("Error: The length of the uncompessed peaks list (%d) does not match the length stored in the file (%d)!\n", lk_Bytes.size(), li_CompressedLength);
                    exit(1);
                }
                quint32 lui_Length = mk_ScanStack.front().mr_Spectrum.mi_PeaksCount * 2 * QVariant(mk_XmlPath.first().second["precision"]).toInt() / 8;
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
            else
            {
                // uncompressed peaks
                lk_Bytes = QByteArray::fromBase64(as_Text.toAscii());
            }
            k_ScanIterator::convertValues(lk_Bytes, mk_ScanStack.front().mr_Spectrum.mi_PeaksCount, QVariant(mk_XmlPath.first().second["precision"]).toInt(), mk_XmlPath.first().second["byteOrder"] == "network", lk_Targets);
            mk_ScanStack.front().mr_Spectrum.md_MzValues_ = lk_Targets[0];
            mk_ScanStack.front().mr_Spectrum.md_IntensityValues_ = lk_Targets[1];
        }
    }
    else if (as_Tag == "scan")
    {
        if (mk_ScanIterator.isInterestingScan(mk_ScanStack.front()))
        {
            bool lb_Continue = true;
            mk_ScanIterator.handleScan(mk_ScanStack.front(), lb_Continue);
            if (!lb_Continue)
                cancelParsing();
        }
        mk_ScanStack.pop_front();
    }
}
