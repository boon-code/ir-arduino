#pragma once

/* **********************************************************************
 *                          PUBLIC INTERFACE
 * **********************************************************************/

/*! Declare pin and pin operations */
#define PIN_MAKE(pk,pn,op) _PIN_HELPER(DDR##pk, PORT##pk, PIN##pk, P##pk##pn, op)

/*! Set pin to specific level */
#define PIN_SET_LEVEL(name, level) do { \
	if (level) { \
		PIN_SET(name); \
	} else { \
		PIN_CLEAR(name); \
	} \
} while (0)

/*! Set pin to high */
#define PIN_SET(name) name(SET_PIN)
/*! Set pin to low */
#define PIN_CLEAR(name) name(CLEAR_PIN)
/*! Set pin direction to output */
#define PIN_DIR_OUT(name) name(OUTPUT)
/*! Set pin direction to input */
#define PIN_DIR_IN(name) name(INPUT)
/*! Get pin value (read PINX register) */
#define PIN_GET_IN(name) name(GET_IN)
/*! Get pin configuration (read PORTX register) */
#define PIN_GET_OUT(name) name(GET_OUT)
/*! Get pin direction */
#define PIN_GET_DIR(name) name(GET_DIR)

#define PREG_PORT(name) name(REG_PORT)
#define PREG_PIN(name) name(REG_PIN)
#define PREG_DDR(name) name(REG_DDR)

/* **********************************************************************
 *                                INTERNAL
 * **********************************************************************/

#define _PIN_HELPER(ddr,port_o,port_i,pin,op) _PIN_OP_##op(ddr, port_o, port_i, pin)

/* Generic Pin Operations */
#define _PIN_GOP_SET(var, pin) do { \
        (var) |= (1 << (pin)); \
} while (0)

#define _PIN_GOP_CLEAR(var, pin) do { \
        (var) &= ~(1 << (pin)); \
} while (0)

#define _PIN_GOP_TEST(var, pin) ((var) & (1 << (pin)) == 0 ? 0 : 1)

/* Pin Functions */
#define _PIN_OP_SET_PIN(ddr,port_o,port_i,pin) _PIN_GOP_SET(port_o, pin)
#define _PIN_OP_CLEAR_PIN(ddr,port_o,port_i,pin) _PIN_GOP_CLEAR(port_o, pin)
#define _PIN_OP_OUTPUT(ddr,port_o,port_i,pin) _PIN_GOP_SET(ddr, pin)
#define _PIN_OP_INPUT(ddr,port_o,port_i,pin) _PIN_GOP_CLEAR(ddr, pin)
#define _PIN_OP_GET_IN(ddr,port_o,port_i,pin) _PIN_GOP_TEST(port_i, pin)
#define _PIN_OP_GET_OUT(ddr,port_o,port_i,pin) _PIN_GOP_TEST(port_o, pin)
#define _PIN_OP_GET_DIR(ddr,port_o,port_i,pin) _PIN_GOP_TEST(ddr, pin)
#define _PIN_OP_REG_PORT(ddr,port_o,port_i,pin) (port_o)
#define _PIN_OP_REG_PIN(ddr,port_o,port_i,pin) (port_i)
#define _PIN_OP_REG_DDR(ddr,port_o,port_i,pin) (ddr)
