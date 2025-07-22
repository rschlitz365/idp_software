#ifndef GLOBALDEFINES_H
#define GLOBALDEFINES_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

enum IdpDataType
  { UnknownDT, SeawaterDT, AerosolsDT, PrecipitationDT, CryosphereDT };

enum ParamSamplingSystem
  { UnknownSS,
    SensorSS,
    BottleSS, PumpSS, FishSS, BoatPumpSS, UwaySS, SubicePumpSS,
    HivolSS, LowvolSS, CoarseImpactorSS, FineImpactorSS,
    AutoSS, ManSS,
    GrabSS, CorerSS, MeltpondPumpSS,
    UnifiedSS
  };


#endif   // GLOBALDEFINES_H
