#ifndef SDI12_LOG_H
#define SDI12_LOG_H

#include "app_config.h"
#include "struct.h"
#include "data_store.h"

namespace SDI12Log
{
	struct Entry
	{
		unsigned long long timestamp;

		char response[64];
	};

	RetResult add(char *data);

    DataStore<Entry>* get_store();

	void print(const Entry *data);
}

#endif