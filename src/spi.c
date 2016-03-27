#include <avr/io.h>

#include "spi.h"

#ifdef SPI_USE_CS
# define spi_chip_select()   PIN_CLEAR(SPI_SS)
# define spi_chip_release()  PIN_SET(SPI_SS)
#else
# define spi_chip_select()
# define spi_chip_release()
#endif

void spi_init_master(void)
{
	uint8_t tmp = 0;

#ifdef SPI_USE_CS
	PIN_DIR_OUT(SPI_SS);
#else /* Activate pull-ups to prevent multi-master detection */
	PIN_DIR_IN(SPI_SS);
	PIN_SET(SPI_SS);
#endif
	PIN_DIR_OUT(SPI_MOSI);
	PIN_DIR_OUT(SPI_SCLK);
	PIN_DIR_IN(SPI_MISO);

	spi_chip_release();

	// enable SPI, master, clock rate F_OSC/16, 
	// CPOL = 0 CPHA = 0
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1);

	// dummy read...
	tmp = SPSR;
	tmp = SPDR;
}

void spi_shutdown(void)
{
	spi_chip_release();

	SPCR = 0x00;

#ifdef SPI_USE_CS
	PIN_DIR_IN(SPI_SS);
	PIN_CLEAR(SPI_SS);
#endif
	PIN_DIR_IN(SPI_SCLK);
	PIN_CLEAR(SPI_SCLK);
	PIN_DIR_IN(SPI_MOSI);
	PIN_CLEAR(SPI_MOSI);
	PIN_DIR_IN(SPI_MISO);
	PIN_CLEAR(SPI_MISO);
}

uint8_t spi_btransfer(const uint8_t data)
{
	spi_chip_select();

	//Start new transmission
	SPDR = data;

	// wait until finished
	while(!(SPSR & (1 << SPIF)));

	spi_chip_release();

	return SPDR;
}

uint16_t spi_wtransfer(const uint16_t data)
{
	uint8_t data_lo = (uint8_t)data;
	uint8_t data_hi = (uint8_t)(data >> 8);

	spi_chip_select();

	SPDR = data_hi;

	while(!(SPSR & (1 << SPIF)));

	data_hi = SPDR;

	SPDR = data_lo;

	while(!(SPSR & (1 << SPIF)));

	data_lo = SPDR;

	spi_chip_release();

	return (data_hi << 8) | (data_lo);
}
