#ifndef WIPY_H
#define WIPY_H

#include <inttypes.h>

#define BOARD_NAME F("WiPy")
#define TINY_GSM_MODEM_SIM7000

/******************************************************************************
 * Pins
 *****************************************************************************/
/** GSM module */
#define PIN_GSM_TX 21
#define PIN_GSM_RX 22
#define PIN_GSM_PWR_KEY 37  // Not used in WiPy+SIM7000
#define PIN_GSM_RESET 37	// Not used in WiPy+SIM7000
#define PIN_GSM_POWER_ON 13 // POWER KEY active high

/** I2C pins for the main I2C bus */
#define PIN_I2C1_SDA 12
#define PIN_I2C1_SCL 2

/** I2C pins for the I2C-SDI12 adapter */
#define PIN_SDI12_I2C1_SDA 33 // Nano pin A4
#define PIN_SDI12_I2C1_SCL 32 // Nano pin A5 

/** Water quality sensor and SDI12 adapter power is controlled from the same pin */
#define PIN_WATER_QUALITY_PWR 27

/** Ultrasonic Water Level sensor */
#define PIN_WATER_LEVEL_PWR 19
#define PIN_WATER_LEVEL_ANALOG 36

#endif