#include "wdog_timer.h"
#include <stdlib.h>
#include <util/atomic.h>

/* Watch Dog Timer Implementation */

#define WDT_CONFIG_MASK (_BV(WDIE) | _BV(WDE))

#define wdt_config_locked(config,value) \
__asm__ __volatile__ ( \
	"wdr" "\n\t" \
	"sts %0,%1" "\n\t" \
	"sts %0,%2" "\n\t" \
 \
	: /* no outputs */ \
	: "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
	"r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)), \
	"r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) | \
             (config & WDT_CONFIG_MASK) | (value & 0x07)) ) \
	: "r0" \
)

#define WDT_STATE_OFF 0
#define WDT_STATE_RUNNING 1


static volatile uint16_t *bootKeyPtr = (volatile uint16_t *)0x0800;

static struct {
	volatile uint16_t counter;
	volatile uint8_t state;
	volatile wdt_callback_t callback;
} wdt_state = {
	.counter = 0,
	.state = WDT_STATE_OFF,
	.callback = NULL,
};

static void clear_wdt_irq (void);

static void clear_wdt_irq (void)
{
	WDTCSR |= _BV(WDIF); /* clear irq */
}

extern void wd_tick (void);

ISR(WDT_vect)
{
#if DEBUG_LEVEL == 2
	wd_tick();
#endif
	if (wdt_state.counter == 0) {
		wdt_config_locked(0, 0);
		wdt_state.state = WDT_STATE_OFF;
		if (wdt_state.callback != NULL) {
			wdt_state.callback();
		}
	} else {
		--wdt_state.counter;
	}
}

void wdt_enter_bootloader (void)
{
	cli();
	MCUCR |= (1 << IVCE);
	MCUCR = (1 << IVSEL);
	bootKeyPtr[0] = 0x7777;
	wdt_enable(WDTO_250MS);
	while (1) {}
}

int8_t wdt_schedule (uint8_t wdto, uint16_t count, wdt_callback_t cb)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (wdt_state.state != WDT_STATE_OFF)
			return -EBUSY;
	        wdt_state.counter = count;
		wdt_state.callback = cb;
		wdt_state.state = WDT_STATE_RUNNING;
		wdt_config_locked(_BV(WDIE), wdto);
	}

	return 0;
}

void wdt_schedule_replace (uint8_t wdto, uint16_t count, wdt_callback_t cb)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		wdt_state.counter = count;
		wdt_state.callback = cb;
		wdt_state.state = WDT_STATE_RUNNING;
		wdt_config_locked(0, 0);
		clear_wdt_irq();
		wdt_config_locked(_BV(WDIE), wdto);
	}
}

void wdt_cancel (void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		wdt_state.counter = 0;
		wdt_state.callback = NULL;
		wdt_state.state = WDT_STATE_OFF;
		wdt_config_locked(0, 0);
		clear_wdt_irq();
	}
}

uint8_t wdt_running (void)
{
	uint8_t ret = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		ret  = (wdt_state.state == WDT_STATE_RUNNING) ?
			1 : 0;
	}

	return ret;
}
