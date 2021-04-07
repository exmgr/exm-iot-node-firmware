#ifndef WIPY3_H
#define WIPY3_H

#include <inttypes.h>

#define BOARD_NAME F("WiPy3")

#define TINY_GSM_MODEM_SIM7000

/******************************************************************************
 * Pins
 *****************************************************************************/
/** GSM module */
#define PIN_GSM_TX 22
#define PIN_GSM_RX 21
#define PIN_GSM_PWR_KEY 5 // Power control
#define PIN_GSM_RESET 15	// TODO: Randomly set, must fix 
#define PIN_GSM_POWER_ON 15 // TODO: Randomly set, must fix

/** I2C pins for the main I2C bus */
#define PIN_I2C1_SDA 32
#define PIN_I2C1_SCL 26

/** I2C pins for the I2C-SDI12 adapter */
#define PIN_SDI12_I2C1_SDA 12 // Nano pin A4
#define PIN_SDI12_I2C1_SCL 13 // Nano pin A5 

/** Water quality sensor and SDI12 adapter power is controlled from the same pin */
#define PIN_WATER_QUALITY_PWR 25

/** Ultrasonic Water Level sensor */
#define PIN_WATER_LEVEL_PWR 14
#define PIN_WATER_LEVEL_ANALOG 36

#endif