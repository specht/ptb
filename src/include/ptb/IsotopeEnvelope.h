/*
Copyright (c) 2009-2010 Michael Specht

This file is part of qTrace.

qTrace is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SimQuant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with qTrace.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <QtCore>


// tk_IsotopeEnvelope:
// list of isotope peaks (starting at A+0):
//   (abundance, average mass shift)
typedef QList<QPair<double, double> > tk_IsotopeEnvelope;
typedef QHash<QString, QList<double> > tk_ModifiedAbundances;


class k_IsotopeEnvelope
{
public:
    k_IsotopeEnvelope(tk_ModifiedAbundances ak_ModifiedAbundances = tk_ModifiedAbundances());
    virtual ~k_IsotopeEnvelope();
    tk_IsotopeEnvelope isotopeEnvelopeForComposition(QHash<QString, int> ak_Composition);
    tk_IsotopeEnvelope add(const tk_IsotopeEnvelope& ak_First, const tk_IsotopeEnvelope& ak_Second);
    tk_IsotopeEnvelope normalize(const tk_IsotopeEnvelope& ak_Envelope);
    double massForComposition(QHash<QString, int> ak_Composition);
    
    QHash<QString, int> mk_BaseIsotope;
    QHash<QString, double> mk_BaseIsotopeMass;
    /*
    mk_ElementEnvelopes (multiples of 2):
    (1, 2, 4, 8, 16, 32, ...)
    */
    QHash<QString, QList<tk_IsotopeEnvelope> > mk_ElementEnvelopes;
    QHash<QString, QList<double> > mk_ElementIsotopeMassShift;
};

/*

for clarification:
    
E  I  ABUND. MASS SHIFT
H: 0: 0.9999 0.00000000
H: 1: 0.0001 1.00627675
H: 2: 0.0000 2.00822424
N: 0: 0.9963 0.00000000
N: 1: 0.0037 0.99703489
O: 0: 0.9976 0.00000000
O: 1: 0.0004 1.00421688
O: 2: 0.0021 2.00424578
S: 0: 0.9493 0.00000000
S: 1: 0.0076 0.99938781
S: 2: 0.0429 1.99579614
S: 3: 0.0000 0.00000000
S: 4: 0.0002 3.99501019
C: 0: 0.9893 0.00000000
C: 1: 0.0107 1.00335484
C: 2: 0.0000 2.00324199

*/
