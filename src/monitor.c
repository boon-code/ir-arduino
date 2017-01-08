#include <avr/pgmspace.h>

#include "ir_arduino.h"
#include "monitor.h"
#include "spi.h"


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

static const char STR_DELETE_VALUE[] PROGMEM = "\r                  \r";


static void usb_write_pstr(USB_ClassInfo_CDC_Device_t *vdev, const char *pstr);
static void toggle(USB_ClassInfo_CDC_Device_t *vdev, uint8_t* value, const char* text);
static void pga_cs(unsigned char select);
static void write_to_pga(USB_ClassInfo_CDC_Device_t *vdev, const char *text);
static unsigned char convert_hex_digit(unsigned char ch, unsigned char* out);
static unsigned char is_number(unsigned char ch);
unsigned char read_decimal(USB_ClassInfo_CDC_Device_t *vdev);
static unsigned char usb_getchar(USB_ClassInfo_CDC_Device_t *vdev);


static unsigned char usb_getchar(USB_ClassInfo_CDC_Device_t *vdev)
{
	while (CDC_Device_BytesReceived(vdev) == 0) {
		CDC_Device_USBTask(vdev);
		USB_USBTask();
	}

	return CDC_Device_ReceiveByte(vdev);
}

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
		usb_write_pstr(vdev, STR_TOGGLE_DISABLE);
	} else {
		val = 1;
		usb_write_pstr(vdev, STR_TOGGLE_ENABLE);
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

static void write_to_pga(USB_ClassInfo_CDC_Device_t *vdev, const char *text)
{
	uint8_t left = 0;
	uint8_t right = 0;
	uint16_t tx = 0;

	pga_cs(0);

	usb_write_pstr(vdev, text);
	usb_write_pstr(vdev, STR_RIGHT);
	right = read_decimal(vdev);

	usb_write_pstr(vdev, STR_NL);
	usb_write_pstr(vdev, STR_LEFT);
	left = read_decimal(vdev);

	usb_write_pstr(vdev, STR_NL);
	fprintf(&usb_stream, "PGA Setup (r,l): %hhx, %hhx", right, left);
	CDC_Device_USBTask(vdev);
	USB_USBTask();

	_delay_us(10);
	pga_cs(1);
	tx = (right << 8);
	tx |= left;
	tx = spi_wtransfer(tx);
	pga_cs(0);

	usb_write_pstr(vdev, STR_NL);

	left = tx & 0xff;
	right = (tx >> 8) & 0xff;
	fprintf(&usb_stream, "PGA Setup before (r,l): %hhx, %hhx", right, left);
	CDC_Device_USBTask(vdev);
	USB_USBTask();
	usb_write_pstr(vdev, STR_NL);
	usb_write_pstr(vdev, STR_NL);
}

static unsigned char convert_hex_digit(unsigned char ch, unsigned char* out)
{
	if ((ch >= '0') && (ch <= '9')) {
		*out = (ch - '0');
		return 1;
	} else if ((ch >= 'A') && (ch <= 'F')) {
		*out = (ch - 'A') + 10;
		return 1;
	} else if ((ch >= 'a') && (ch <= 'f')) {
		*out = (ch - 'a') + 10;
		return 1;
	} else {
		return 0;
	}
}

static unsigned char is_number(unsigned char ch)
{
	if ((ch >= '0') && (ch <= '9')) {
		return 1;
	} else {
		return 0;
	}
}

unsigned char read_decimal(USB_ClassInfo_CDC_Device_t *vdev)
{
	unsigned char chs[3];
	unsigned char rx_index = 0;
	unsigned int val = 0;
	unsigned char no_ack = 1;

	while (no_ack) {
		unsigned char rx = usb_getchar(vdev);

		if ((rx == '\n') || (rx == '\r')) {
			no_ack = 0;
		} else if (is_number(rx)) {
			switch (rx_index) {
			case 0:
			case 1:
				chs[rx_index] = rx;
				CDC_Device_SendByte(vdev, rx);
				break;
			case 2:
				if ((val * 10 + (rx - '0')) <= 0xff) {
					chs[rx_index] = rx;
					CDC_Device_SendByte(vdev, rx);
					break;
				}
			default:
				--rx_index;
			}
			++rx_index;
		} else if ((rx_index > 0) && ((rx == 8) || (rx == 0x7f))) {
			chs[rx_index] = 0;
			--rx_index;
		} else if (rx == 0x1b) {
			return 0;
		}

		CDC_Device_USBTask(vdev);
		USB_USBTask();

		usb_write_pstr(vdev, STR_DELETE_VALUE);
		val = 0;
		for(unsigned char i = 0; i < rx_index; ++i) {
			unsigned char ch = chs[i];
			CDC_Device_SendByte(vdev, ch);
			val *= 10;
			val += (ch - '0');
		}
	}

	return (unsigned char) val;
}

void sub_pga(USB_ClassInfo_CDC_Device_t *vdev)
{
	uint8_t exit = 0x0;
	uint8_t zcen = 0;
	uint8_t mute = 0;
	uint8_t cs = 1;

	usb_write_pstr(vdev, STR_CHOOSE_PGA1_ACTION);

	while(!exit) {
		int16_t val;

		PIN_SET_LEVEL(PGA_ZCEN_O, zcen);
		PIN_SET_LEVEL(PGA_MUTE_NO, !mute);
		PIN_SET_LEVEL(PGA_CS_NO, !cs);

		val = usb_getchar(vdev);
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
			usb_write_pstr(vdev, STR_UNSUPPORTED);
			break;

		case '7':
			write_to_pga(vdev, STR_PGA1_WRITE);
			break;

		case 'e':
		case 'E':
			exit = 1;
			break;

		default:
			usb_write_pstr(vdev, STR_CHOOSE_PGA1_ACTION);
			break;
		}
	}

	PIN_SET(PGA_CS_NO);
}

