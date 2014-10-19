/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the ir_arduino project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "ir_arduino.h"
#include <util/delay.h>
#include "wdog_timer.h"
#include "util.h"
#include "pin_io.h"

/*
 * Hardware configuration:
 */

/** IR Input Pin (D4) */
#define PIN_IR_I(op)       PIN_MAKE(D,4,op)

/** INT0: IR Wake (D3) */
#define PIN_IR_WAKE_I(op)  PIN_MAKE(D,0,op)

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = {
	.Config = {
		.ControlInterfaceNumber = INTERFACE_ID_CDC_CCI,
		.DataINEndpoint = {
			.Address = CDC_TX_EPADDR,
			.Size    = CDC_TXRX_EPSIZE,
			.Banks   = 1,
		},
		.DataOUTEndpoint = {
			.Address = CDC_RX_EPADDR,
			.Size    = CDC_TXRX_EPSIZE,
			.Banks   = 1,
		},
		.NotificationEndpoint = {
			.Address = CDC_NOTIFICATION_EPADDR,
			.Size    = CDC_NOTIFICATION_EPSIZE,
			.Banks   = 1,
		},
	},
};

static FILE usb_stream;

#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL 1
#endif

#define __trace_log(level,...) do { \
	fprintf(&usb_stream, "[" #level "] " __VA_ARGS__); \
} while (0)

#if DEBUG_LEVEL == 2
# define info(...) __trace_log(info, __VA_ARGS__)
# define dbg(...) __trace_log(dbg, __VA_ARGS__)
# warning Debug level is DEBUG
#elif DEBUG_LEVEL == 1
# define info(...) __trace_log(info, __VA_ARGS__)
# define dbg(...)
# warning Debug level is INFO
#elif DEBUG_LEVEL == 0
# define info(...)
# define dbg(...)
# warning Debug level is OFF
#else
# error Debug level (DEBUG_LEVEL) is invalid (or not set)
#endif


static struct {
	uint8_t received;
	uint8_t got_events;
	uint8_t code[4];
	uint16_t stamps[256];
} g_ir = {
	.received = 0,
	.got_events = 0,
	.code = {0, 0, 0, 0},
};

//static uint8_t g_ir_received = 0;
//static uint8_t g_ir_got_events = 0;
//static uint16_t g_ir_stamps[256];

static void ir_test_main(void);
static void ir_disable(void);
static void ir_enable(void);
static void blink(uint8_t max);


ISR(TIMER1_CAPT_vect)
{
	TCNT1 = 0x0000;
	g_ir.stamps[g_ir.received] = ICR1;
	++g_ir.received;
}

ISR(TIMER1_OVF_vect)
{
	if (g_ir.received > 0) {
		g_ir.got_events = 1;
		ir_disable();
	}
}

static void ir_disable(void)
{
	/* Disable CAPT */
	TIMSK1 &= ~(1 << ICIE1);
}

static void ir_enable(void)
{
	g_ir.received = 0;
	g_ir.got_events = 0;
	/* Enable CAPT: */
	TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
}

static void ir_initialize(void)
{
	/*        filter=1      rising edge    clk/1 */
	TCCR1B = (1 << ICNC1) | (1 << ICES1) | 0x1;
	ir_enable();
}

static uint8_t ir_evaluate(void)
{
	uint8_t index = 0;
	uint8_t i;
	uint8_t j;

	if (g_ir.received < 32) {
		return 0;
	}

	for (i = 0; i < 4; ++i) {
		g_ir.code[i] = 0;
		for (j = 0; j < 8; ++j) {
			if (g_ir.stamps[index] > 25000) {
				g_ir.code[i] |= (1 << j);
			}
			++index;
		}
	}

	return 1;
}

static void ir_test_main(void)
{
	if (g_ir.got_events) {
		for (uint8_t i = 0; i < g_ir.received; ++i) {
			info("stamp [%hhu]: %hu\r\n", i, g_ir.stamps[i]);
			CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
			USB_USBTask();
		}
		info("Processed %hhd stamps\r\n", g_ir.received);
		if (ir_evaluate()) {
			fprintf(&usb_stream, "%hhx%hhx%hhx%hhx\r\n", g_ir.code[0], g_ir.code[1], g_ir.code[2], g_ir.code[3]);
		}
		ir_enable();
	}
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	VirtualSerial_CDC_Interface.State.LineEncoding.BaudRateBPS = 9600; /* Reset variable to some default value */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &usb_stream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	ir_initialize();

	while (1) {
		ir_test_main();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif
	PIN_CLEAR(PIN_IR_I);
	PIN_DIR_IN(PIN_IR_I);

	PIN_CLEAR(PIN_IR_WAKE_I);
	PIN_DIR_IN(PIN_IR_WAKE_I);

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_Suspend (void)
{
	blink(2);
}

void EVENT_USB_Device_WakeUp (void)
{
	LEDs_SetAllLEDs(LEDS_LED1 | LEDS_LED2);
}

//void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
//{
//	fprintf(&usb_stream, "H2D: %lx\r\n", (long)CDCInterfaceInfo->State.ControlLineStates.HostToDevice);
//	fprintf(&usb_stream, "D2H: %lx\r\n", (long)CDCInterfaceInfo->State.ControlLineStates.DeviceToHost);
//}

/** Event handler for the CDC Class driver Line Encoding Changed event.
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{}

static void blink (uint8_t max)
{
	uint8_t i;

	for (i = 0; i < max; ++i) {
		LEDs_SetAllLEDs(LEDS_LED1 | LEDS_LED2 | LEDS_LED3);
		_delay_ms(250);
		LEDs_SetAllLEDs(0);
		_delay_ms(250);
	}
}
