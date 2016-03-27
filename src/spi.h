#pragma once

#include "pin_io.h"

#define SPI_MISO(op)     PIN_MAKE(B,3,op)
#define SPI_MOSI(op)     PIN_MAKE(B,2,op)
#define SPI_SCLK(op)     PIN_MAKE(B,1,op)
#define SPI_SS(op)       PIN_MAKE(B,0,op)


void spi_init_master(void);
void spi_shutdown(void);
uint8_t spi_btransfer(const uint8_t data);
uint16_t spi_wtransfer(const uint16_t data);
