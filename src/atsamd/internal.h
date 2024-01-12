#ifndef __ATSAMD_INTERNAL_H
#define __ATSAMD_INTERNAL_H
// Local definitions for atsamd code

#include <stdint.h> // uint32_t
#include "autoconf.h" // CONFIG_MACH_SAMD21A

#if CONFIG_MACH_SAMC21
#include "samc21.h"
#elif CONFIG_MACH_SAMD21
#include "samd21.h"
#elif CONFIG_MACH_SAMD51
#include "samd51.h"
#elif CONFIG_MACH_SAME51
#include "same51.h"
#elif CONFIG_MACH_SAME54
#include "same54.h"
#endif

#define GPIO(PORT, NUM) (((PORT)-'A') * 32 + (NUM))
#define GPIO2PORT(PIN) ((PIN) / 32)
#define GPIO2BIT(PIN) (1<<((PIN) % 32))

#define GET_FUSE(REG)                                           \
    ((*((uint32_t*)(REG##_ADDR)) & (REG##_Msk)) >> (REG##_Pos))

void enable_pclock(uint32_t pclk_id, int32_t pm_id); // clock.c
uint32_t get_pclock_frequency(uint32_t pclk_id); // clock.c
void gpio_peripheral(uint32_t gpio, char ptype, int32_t pull_up); // gpio.c

Sercom * sercom_enable_pclock(uint32_t sercom_id); // sercom.c
Sercom * sercom_get_by_id(uint32_t sercom_id); // sercom.c
uint32_t sercom_get_pclock_frequency(uint32_t sercom_id); // sercom.c

void set_sercom_interrupt(uint32_t sercom_id, void (*handler)(uint32_t sercom_id)); // sercom.c
void clear_sercom_interrupt(uint32_t sercom_id); // sercom.c
uint32_t sercom_serial_pins(uint32_t sercom_id, uint8_t tx_pin, uint8_t rx_pin);
uint32_t sercom_usart_pins(uint32_t sercom_id, uint8_t mode); // sercom.c
uint32_t sercom_spi_pins(uint32_t sercom_id); // sercom.c
void sercom_i2c_pins(uint32_t sercom_id); // sercom.c

#endif // internal.h
