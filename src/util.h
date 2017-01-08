#pragma once

#define PSAVE_MCU_LOCKED() do { \
        sleep_enable(); \
        set_sleep_mode(SLEEP_MODE_PWR_SAVE); \
        sei(); \
        sleep_cpu(); \
        sleep_disable(); \
} while (0)

#define PDOWN_MCU_LOCKED() do { \
        sleep_enable(); \
        set_sleep_mode(SLEEP_MODE_PWR_DOWN); \
        sei();			     \
        sleep_cpu(); \
        sleep_disable(); \
} while (0)
