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

#include <ptb/MzDataHandler.h>
#include <ptb/ScanIterator.h>


k_MzDataHandler::k_MzDataHandler(k_ScanIterator& ak_ScanIterator)
    : k_XmlHandler(ak_ScanIterator)
    , mr_pScan(new r_Scan())
{
}


k_MzDataHandler::~k_MzDataHandler()
{
}


bool k_MzDataHandler::startElement(const QString &namespaceURI, const QString &localName,
                                   const QString &qName, const QXmlAttributes &attributes)
{
    if (mb_Cancelled)
        return false;
    
    if (qName == "precursor")
        mr_pScan->mk_Precursors.push_back(r_Precursor());
        
    return k_XmlHandler::startElement(namespaceURI, localName, qName, attributes);
}


void k_MzDataHandler::handleElement(const QString& as_Tag, const tk_XmlAttributes& ak_Attributes, const QString as_Text)
{
    if (as_Tag == "spectrumInstrument")
        mr_pScan->mi_MsLevel = QVariant(ak_Attributes["msLevel"]).toInt();
    else if (as_Tag == "cvParam")
    {
        if (ak_Attributes["name"] == "TimeInMinutes")
            mr_pScan->md_RetentionTime = QVariant(ak_Attributes["value"]).toDouble();
        else if (ak_Attributes["name"] == "Filter")
            mr_pScan->ms_FilterLine = ak_Attributes["value"];
        else if (ak_Attributes["name"] == "ScanMode")
        {
            QString ls_ScanMode = ak_Attributes["value"];
            if (ls_ScanMode == "MassScan")
                mr_pScan->me_Type = r_ScanType::MSn;
            else if (ls_ScanMode == "SingleIonMonitoring")
                mr_pScan->me_Type = r_ScanType::SIM;
            else
                mr_pScan->me_Type = r_ScanType::Unknown;
        }
        else if (ak_Attributes["name"] == "MassToChargeRatio")
        {
            mr_pScan->mk_Precursors.last().md_Mz = QVariant(ak_Attributes["value"]).toDouble();
        }
        else if (ak_Attributes["name"] == "ChargeState")
        {
            if (mr_pScan->mk_Precursors.last().mi_ChargeState != 0)
            {
                // one precursor already exists, add a new one
                r_Precursor lr_Precursor = mr_pScan->mk_Precursors.last();
                mr_pScan->mk_Precursors.push_back(lr_Precursor);
            }
            mr_pScan->mk_Precursors.last().mi_ChargeState = QVariant(ak_Attributes["value"]).toInt();
        }
    }
    else if (as_Tag == "data" && mk_ScanIterator.isInterestingScan(*(mr_pScan.data())))
    {
        int li_PeaksCount = 0;
        double* ld_Values_;
        li_PeaksCount = QVariant(ak_Attributes["length"]).toInt();
        QList<double*> lk_Targets;
        lk_Targets << ld_Values_;
        k_ScanIterator::convertValues(QByteArray::fromBase64(as_Text.toAscii()), 
            li_PeaksCount, QVariant(ak_Attributes["precision"]).toInt(), 
            ak_Attributes["endian"] != "little", lk_Targets);
        ld_Values_ = lk_Targets[0];
        if (mk_XmlPath[1].first == "mzArrayBinary")
        {
            mr_pScan->mr_Spectrum.mi_PeaksCount = li_PeaksCount;
            mr_pScan->mr_Spectrum.md_MzValues_ = ld_Values_;
        }
        else if (mk_XmlPath[1].first == "intenArrayBinary")
        {
            mr_pScan->mr_Spectrum.mi_PeaksCount = li_PeaksCount;
            mr_pScan->mr_Spectrum.md_IntensityValues_ = ld_Values_;
        }
        else
            printf("Warning: don't know where to put base64 encoded floating point values!\n");
    }
    else if (as_Tag == "spectrum")
    {
        mr_pScan->ms_Id = ak_Attributes["id"];
        
        bool lb_InterestingScan = mk_ScanIterator.isInterestingScan(*(mr_pScan.data()));
        mk_ScanIterator.progressFunction(mr_pScan->ms_Id, lb_InterestingScan);

        if (lb_InterestingScan)
        {
            bool lb_Continue = true;
            mk_ScanIterator.handleScan(*(mr_pScan.data()), lb_Continue);
            if (!lb_Continue)
                cancelParsing();
        }
        // create a fresh scan
        mr_pScan = QSharedPointer<r_Scan>(new r_Scan());
    }
}
