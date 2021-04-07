#ifndef FEATHER_H
#define FEATHER_H

#include <inttypes.h>

#define BOARD_NAME "Feather"

/******************************************************************************
 * Pins
 *****************************************************************************/
/** GSM module */
#define PIN_GSM_TX 16
#define PIN_GSM_RX 17
#define PIN_GSM_PWR_KEY 21 // Power control

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
