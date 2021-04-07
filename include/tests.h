#ifndef TESTS_H
#define TESTS_H
#include "struct.h"

namespace Tests
{
	/** Available tests. Enum vals are "test_ids" and must start from 0.
	 * Enum val maps all properties (names, funcs etc) to their test */
	enum TestId
	{
		RTC_FROM_GSM,
		DATA_STORE,
		WAKEUP_TIMES,
		DEVICE_CONFIG
	};

	RetResult rtc_from_gsm();

	RetResult sensor_data_logging();

	RetResult data_store();

	RetResult wakeup_times();

	RetResult device_config();

	void run(TestId tests[], int count);

	void run_all();

} // Tests

#endif