#pragma once

#include "ir_arduino.h"
#include "pin_io.h"

#define PGA_ZCEN_O(op)     PIN_MAKE(B,6,op)
#define PGA_CS_NO(op)      PIN_MAKE(B,5,op)
#define PGA_MUTE_NO(op)    PIN_MAKE(B,4,op)

void sub_pga(USB_ClassInfo_CDC_Device_t *vdev);
