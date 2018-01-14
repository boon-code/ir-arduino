#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

MCU          = atmega32u4
ARCH         = AVR8
BOARD        = ARDUINO_MICRO
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = ir_arduino
SRC          = $(TARGET).c \
               spi.c \
               Descriptors.c \
               wdog_timer.c \
               $(LUFA_SRC_USB) \
               $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ../../lufa/LUFA
DLEVEL      ?= 0
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/ -DDEBUG_LEVEL=$(DLEVEL)
LD_FLAGS     = -Wl,-u,vfprintf -lprintf_flt -lm
AVRDUDE_PROGRAMMER :=  avr109
AVRDUDE_PORT       :=  /dev/ttyARDUINO

# Default target
all:

program: $(TARGET).hex $(MAKEFILE_LIST)
	sh ./program.sh

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
