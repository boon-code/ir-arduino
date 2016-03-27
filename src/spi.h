#pragma once

#include "pin_io.h"

#define SPI_MISO(op)     PIN_MAKE(B,3,op)
#define SPI_MOSI(op)     PIN_MAKE(B,2,op)
#define SPI_SCLK(op)     PIN_MAKE(B,1,op)
#define SPI_SS(op)       PIN_MAKE(B,0,op)


void spi_init_master(void);
void spi_shutdown(void);
unsigned char spi_btransfer(const unsigned char data);
unsigned short spi_wtransfer(const unsigned short data);
