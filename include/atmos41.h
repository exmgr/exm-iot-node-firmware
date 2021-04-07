#ifndef ATMOS41_H
#define ATMOS41_H

#include "atmos41_data.h"

namespace Atmos41
{
	RetResult init();
	RetResult on();
	RetResult off();

    RetResult measure(Atmos41Data::Entry *data);
	RetResult measure_dummy(Atmos41Data::Entry *data);
	RetResult measure_log();
}

#endif