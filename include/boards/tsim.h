#ifndef TSIM_H
#define TSIM_H

#include <inttypes.h>

#define BOARD_NAME F("TSim")

#define TINY_GSM_MODEM_SIM7000

/******************************************************************************
 * Pins
 *****************************************************************************/
/** Pin used by switch that is used to enter config mode on boot.
 * Pin can be one that is already used by another function because it is used
 * briefly on boot (has pull-ups turned on for a short time) to check whether 
 * to enter config mode, and then released */
#define PIN_CONFIG_MODE_BTN 15

/** Indicator LED. Used by other components as GPIO, could cause problems. */
#define PIN_LED 12

/** GSM module */
#define PIN_GSM_TX 27
#define PIN_GSM_RX 26
#define PIN_GSM_PWR_KEY 4
#define PIN_GSM_RESET 5
#define PIN_GSM_POWER_ON 23
#define PIN_GSM_DTR 25
 
/** I2C pins for the main I2C bus */
#define PIN_I2C1_SDA 21
#define PIN_I2C1_SCL 22

/** Data pin for SDI12 sensors */
#define PIN_SDI12_DATA GPIO_NUM_32 //

/** Water sensors (quality & lev
 * el) and SDI12 adapter power is controlled from the same pin */
#define PIN_WATER_SENSORS_PWR GPIO_NUM_12 // ????

/** Water level ANALOG version input */
#define PIN_WATER_LEVEL_ANALOG 14 // ????
/** Water level PWM version input */
#define PIN_WATER_LEVEL_PWM 14 // ???? 
/** Water level RX pin for serial input */
#define PIN_WATER_LEVEL_SERIAL_RX 14

/** Battery ADC input */
#define PIN_ADC_BAT 35 
/** Solar ADC input */
#define PIN_ADC_SOLAR 36 

/** Data request pin in FineOffset UART mode, makes weather station output data */
#define PIN_FO_UART_REQ_DATA 32

/** RX pin for FineOffset in UART mode */
#define PIN_FO_UART_RX 14

/** RFM9x module */
/** DI0 interrupt pin */
#define PIN_RF_DI0 GPIO_NUM_33

/** RF module SPI */
#define PIN_RF_MISO 19
#define PIN_RF_MOSI 23
#define PIN_RF_SCK 18
#define PIN_RF_SS 5

/* Lightning sensor IRQ pin */
#define PIN_LIGHTNING_IRQ GPIO_NUM_33

/* Water presence sensor  */
#define PIN_WATER_PRESENCE GPIO_NUM_34

#endif