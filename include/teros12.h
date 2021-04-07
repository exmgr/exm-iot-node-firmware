#ifndef TEROS12_H
#define TEROS12_H

#include "soil_moisture_data.h"

namespace Teros12
{
    RetResult init();

    RetResult measure(SoilMoistureData::Entry *data);

    RetResult log();
}

#endif