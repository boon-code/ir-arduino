#include <avr/pgmspace.h>

#include "ir_arduino.h"
#include "monitor.h"


static const char STR_CHOOSE_PGA1_ACTION[] PROGMEM = " Choose PGA1 action:\r\n  1) Toggle Zero Crossing Enabled\r\n  2) Toggle Mute\r\n  3) Toggle Chip Select\r\n  4) Pulse MOSI\r\n  5) Pulse SCLK\r\n  6) Read MISO\r\n  7) Set attenuation (Normal Operation)\r\n  e) Exit\r\n";

static const char STR_TOGGLE_ENABLE[] PROGMEM = " switched high \r\n";
static const char STR_TOGGLE_DISABLE[] PROGMEM = " switched low \r\n";

static const char STR_ZCEN_TEXT[] PROGMEM = "  Zero Crossing ";
static const char STR_MUTE_TEXT[] PROGMEM = "  Mute ";
static const char STR_CS_TEXT[] PROGMEM = "  Chip Select ";
static const char STR_MOSI[] PROGMEM = "  Mosi ";
static const char STR_SCK[] PROGMEM = "  Sck ";
static const char STR_PULSE_FOR_2_SEC[] PROGMEM = " pulsed high now\r\n";
static const char STR_RESET_PULSE[] PROGMEM = " resetted\r\n";
static const char STR_MISO_HIGH[] PROGMEM = "  Miso is high\r\n";
static const char STR_MISO_LOW[] PROGMEM = "  Miso is low\r\n";
static const char STR_PGA1_WRITE[] PROGMEM = "  Enter decimal value for pga1\r\n";
static const char STR_PGA2_WRITE[] PROGMEM = "  Enter decimal value for pga2\r\n";
static const char STR_LEFT[] PROGMEM = "  left:\r\n";
static const char STR_RIGHT[] PROGMEM = "  right:\r\n";
static const char STR_PGA_SETUP[] PROGMEM = "  pga set up to (r,l): ";
static const char STR_PGA_WAS_SET[] PROGMEM = "  pga has been set to (r, l) before: ";
static const char STR_SEPERATOR[] PROGMEM = ", ";
static const char STR_UNSUPPORTED[] PROGMEM = "  action unsupported\r\n";
static const char STR_NL[] PROGMEM = "\r\n";


static void usb_write_pstr(USB_ClassInfo_CDC_Device_t *vdev, const char *pstr)
{
	uint8_t ch = pgm_read_byte(pstr);
	uint8_t count = 0;

	while (ch != 0) {
		CDC_Device_SendByte(vdev, ch);
		++pstr;
		ch = pgm_read_byte(pstr);

		++count;
		if (count >= 20) {
			count = 0;
			CDC_Device_USBTask(vdev);
			USB_USBTask();
		}
	}

	CDC_Device_USBTask(vdev);
	USB_USBTask();
}

static void toggle(USB_ClassInfo_CDC_Device_t *vdev, uint8_t* value, const char* text)
{
	uint8_t val = *value;

	usb_write_pstr(vdev, text);
	if (val) {
		val = 0;
		usb_write_pstr(STR_TOGGLE_DISABLE);
	} else {
		val = 1;
		usb_write_pstr(STR_TOGGLE_ENABLE);
	}
	*value = val;
}

static void pga_cs(unsigned char select)
{
	_delay_us(1);
	if(select) {
		PIN_CLEAR(PGA_CS_NO);
	} else {
		PIN_SET(PGA_CS_NO);
	}
	_delay_us(1);
}

void write_to_pga(USB_ClassInfo_CDC_Device_t *vdev, const char *text)
{
	uint8_t left = 0;
	uint8_t right = 0;
	uint16_t tx = 0;

	pga_cs(0);

	usb_write_pstr(vdev, text);
	usb_write_pstr(vdev, STR_RIGHT);
	right = read_decimal();

	usb_write_pstr(vdev, STR_NL);
	usb_write_pstr(vdev, STR_LEFT);
	left = read_decimal();
	
	usb_write_pstr(vdev, STR_NL);
	fprintf(usb_stream, "PGA Setup (r,l): %hhx, %hhx", right, left);
	
	_delay_us(10);
	pga_cs(1);
	tx = (right << 8);
	tx |= left;
	tx = spi_wtransfer(tx);
	switch_function(0);
	
	usb_write_pstr(vdev, STR_NL);
	
	left = tx & 0xff;
	right = (tx >> 8) & 0xff;
	fprintf("PGA Setup before (r,l): %hhx, %hhx", right, left);
	usb_write_pstr(vdev, STR_NL);
	usb_write_pstr(vdev, STR_NL);
}
void sub_pga(USB_ClassInfo_CDC_Device_t *vdev)
{
	uint8_t exit = 0x0;
	uint8_t zcen = 0;
	uint8_t mute = 0;
	uint8_t cs = 1;

	spi_init_master();

	usb_write_pstr(STR_CHOOSE_PGA1_ACTION);

	while(!exit) {
		int16_t val;

		PIN_SET_LEVEL(PGA_ZCEN_O, zcen);
		PIN_SET_LEVEL(PGA_MUTE_NO, !mute);
		PIN_SET_LEVEL(PGA_CS_NO, !cs);

		val = CDC_Device_ReceiveByte(vdev);
		switch(val) {
		case '1':
			toggle(vdev, &zcen, STR_ZCEN_TEXT);
			break;

		case '2':
			toggle(vdev, &mute, STR_MUTE_TEXT);
			break;

		case '3':
			toggle(vdev, &cs, STR_CS_TEXT);
			break;

		case '4':
		case '5':
		case '6':
			usb_write_pstr(STR_UNSUPPORTED);
			break;

		case '7':
			write_to_pga(STR_PGA1_WRITE);
			break;

		case 'e':
		case 'E':
			exit = 1;
			break;

		default:
			uart_write_pstr(STR_CHOOSE_PGA1_ACTION);
			break;
		}
	}

	PIN_SET(PGA_CS_NO);
}

