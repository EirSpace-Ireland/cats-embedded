// MS5607 Barometer Device Library
// Author: Luca Jost
// 11.06.2020

#ifndef CATS_MS5607_H
#define CATS_MS5607_H

#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include <stdbool.h>
#include "drivers/spi.h"

/** Exported Defines **/

#define BARO_I2C_TIMEOUT 10

// Commands
#define COMMAND_RESET           0x1E
#define COMMAND_CONVERT_D1_BASE 0x40
#define COMMAND_CONVERT_D2_BASE 0x50
#define COMMAND_ADC_READ        0x00
#define COMMAND_PROM_READ_BASE  0xA0

// Conversion time
#define BARO_CONVERSION_TIME_OSR_BASE 0.6f

/** Exported Types **/

enum ms5607_osr {
  MS5607_OSR_256 = 0,
  MS5607_OSR_512 = 1,
  MS5607_OSR_1024 = 2,
  MS5607_OSR_2048 = 3,
  MS5607_OSR_4096 = 4,
};

enum ms5607_data {
  MS5607_PRESSURE = 1,
  MS5607_TEMPERATURE = 2,
};

typedef struct ms5607_dev {
  // Hardware Configuration
  GPIO_TypeDef *const cs_port;
  uint16_t cs_pin;
  SPI_HandleTypeDef *const spi_handle;
  SPI_BUS *spi_bus;

  // Sensor Configuration
  enum ms5607_osr osr;
  enum ms5607_data data;
  // Calibration coefficients
  uint16_t coefficients[6];

  uint8_t raw_pres[3];
  uint8_t raw_temp[3];
} MS5607;

/** Exported Functions **/

void ms5607_init(MS5607 *dev);

void ms5607_prepare_temp(MS5607 *dev);
void ms5607_prepare_pres(MS5607 *dev);
void ms5607_read_raw(MS5607 *dev);
bool ms5607_get_temp_pres(MS5607 *dev, int32_t *temperature, int32_t *pressure);

#endif
