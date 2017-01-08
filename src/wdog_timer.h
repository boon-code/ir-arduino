#pragma once

#include <avr/sleep.h>
#include <avr/wdt.h>
#include "error_codes.h"

typedef void (*wdt_callback_t)(void);

void wdt_enter_bootloader (void);
int8_t wdt_schedule (uint8_t wdto, uint16_t count, wdt_callback_t cb);
void wdt_schedule_replace (uint8_t wdto, uint16_t count, wdt_callback_t cb);
void wdt_cancel (void);
uint8_t wdt_running (void);

/* Watch Dog Timer Interface */
