#ifndef TCALL_H
#define TCALL_H

#include <inttypes.h>

#define BOARD_NAME F("TCall")

#define TINY_GSM_MODEM_SIM800

/******************************************************************************
 * Pins
 *****************************************************************************/
/** GSM module */
#define PIN_GSM_TX 27
#define PIN_GSM_RX 26
#define PIN_GSM_PWR_KEY 4
#define PIN_GSM_RESET 5
#define PIN_GSM_POWER_ON 23

/** I2C pins for the main I2C bus */
#define PIN_I2C1_SDA 21
#define PIN_I2C1_SCL 22

/** I2C pins for the I2C-SDI12 adapter */
#define PIN_SDI12_I2C1_SDA 32 // Nano pin A4
#define PIN_SDI12_I2C1_SCL 33 // Nano pin A5 

/** Channel toggle pin for toggling between A/B SDI12 channels */
#define PIN_SDI12_CHANNEL_TOGGLE 15 // 

/** Water sensors (quality & level) and SDI12 adapter power is controlled from the same pin */
#define PIN_WATER_SENSORS_PWR 13

/** Water level ANALOG version input */
#define PIN_WATER_LEVEL_ANALOG 14
/** Water level PWM version input */
#define PIN_WATER_LEVEL_PWM 14

/** Battery ADC input */
//#define PIN_ADC_BAT 39 // Custom pcb input
#define PIN_ADC_BAT 35 // TCall

#endif