// Handling of sercom pins
//
// Copyright (C) 2019  Florian Heilmann <Florian.Heilmann@gmx.net>
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/armcm_boot.h"
#include "internal.h" // sercom_enable
#include "command.h" // shutdown
#include "compiler.h" // ARRAY_SIZE
#include "sched.h" // sched_shutdown
#include "usartcmds.h" // UF_HAVE_XCK


/****************************************************************
 * Available sercom blocks
 ****************************************************************/

DECL_ENUMERATION_RANGE("bus", "sercom0", 0, 8);

struct sercom_bus {
    Sercom *sercom;
    uint32_t pclk_id, pm_id;
};

static const struct sercom_bus sercoms[] = {
    { SERCOM0, SERCOM0_GCLK_ID_CORE, ID_SERCOM0 },
    { SERCOM1, SERCOM1_GCLK_ID_CORE, ID_SERCOM1 },
    { SERCOM2, SERCOM2_GCLK_ID_CORE, ID_SERCOM2 },
    { SERCOM3, SERCOM3_GCLK_ID_CORE, ID_SERCOM3 },
#ifdef SERCOM4
    { SERCOM4, SERCOM4_GCLK_ID_CORE, ID_SERCOM4 },
    { SERCOM5, SERCOM5_GCLK_ID_CORE, ID_SERCOM5 },
#endif
#ifdef SERCOM6
    { SERCOM6, SERCOM6_GCLK_ID_CORE, ID_SERCOM6 },
    { SERCOM7, SERCOM7_GCLK_ID_CORE, ID_SERCOM7 },
#endif
};

Sercom *
sercom_enable_pclock(uint32_t sercom_id)
{
    if (sercom_id >= ARRAY_SIZE(sercoms))
        shutdown("Invalid SERCOM bus");
    const struct sercom_bus *sb = &sercoms[sercom_id];
    enable_pclock(sb->pclk_id, sb->pm_id);
    return sb->sercom;
}

Sercom *
sercom_get_by_id(uint32_t sercom_id)
{
    if (sercom_id >= ARRAY_SIZE(sercoms))
        shutdown("Invalid SERCOM bus");
    return sercoms[sercom_id].sercom;
}

uint32_t
sercom_get_pclock_frequency(uint32_t sercom_id)
{
    const struct sercom_bus *sb = &sercoms[sercom_id];
    return get_pclock_frequency(sb->pclk_id);
}


/****************************************************************
 * Pin configurations
 ****************************************************************/

struct sercom_pad {
    uint8_t sercom_id, gpio, pad, ptype;
};

static const struct sercom_pad sercom_pads[] = {
#if CONFIG_MACH_SAMD21
    { 0, GPIO('A', 8), 0, 'C'},
    { 0, GPIO('A', 9), 1, 'C'},
    { 0, GPIO('A', 10), 2, 'C'},
    { 0, GPIO('A', 11), 3, 'C'},
    { 0, GPIO('A', 4), 0, 'D'},
    { 0, GPIO('A', 5), 1, 'D'},
    { 0, GPIO('A', 6), 2, 'D'},
    { 0, GPIO('A', 7), 3, 'D'},

    { 1, GPIO('A', 16), 0, 'C'},
    { 1, GPIO('A', 17), 1, 'C'},
    { 1, GPIO('A', 18), 2, 'C'},
    { 1, GPIO('A', 19), 3, 'C'},
    { 1, GPIO('A', 0), 0, 'D'},
    { 1, GPIO('A', 1), 1, 'D'},
    { 1, GPIO('A', 30), 2, 'D'},
    { 1, GPIO('A', 31), 3, 'D'},

    { 2, GPIO('A', 12), 0, 'C'},
    { 2, GPIO('A', 13), 1, 'C'},
    { 2, GPIO('A', 14), 2, 'C'},
    { 2, GPIO('A', 15), 3, 'C'},
    { 2, GPIO('A', 8), 0, 'D'},
    { 2, GPIO('A', 9), 1, 'D'},
    { 2, GPIO('A', 10), 2, 'D'},
    { 2, GPIO('A', 11), 3, 'D'},

    { 3, GPIO('A', 22), 0, 'C'},
    { 3, GPIO('A', 23), 1, 'C'},
    { 3, GPIO('A', 24), 2, 'C'},
    { 3, GPIO('A', 25), 3, 'C'},
    { 3, GPIO('A', 16), 0, 'D'},
    { 3, GPIO('A', 17), 1, 'D'},
    { 3, GPIO('A', 18), 2, 'D'},
    { 3, GPIO('A', 19), 3, 'D'},
    { 3, GPIO('A', 20), 2, 'D'},
    { 3, GPIO('A', 21), 3, 'D'},
  #ifdef SERCOM4
    { 4, GPIO('B', 12), 0, 'C'},
    { 4, GPIO('B', 13), 1, 'C'},
    { 4, GPIO('B', 14), 2, 'C'},
    { 4, GPIO('B', 15), 3, 'C'},
    { 4, GPIO('B', 8), 0, 'D'},
    { 4, GPIO('B', 9), 1, 'D'},
    { 4, GPIO('B', 10), 2, 'D'},
    { 4, GPIO('B', 11), 3, 'D'},
    { 4, GPIO('A', 12), 0, 'D'},
    { 4, GPIO('A', 13), 1, 'D'},
    { 4, GPIO('A', 14), 2, 'D'},
    { 4, GPIO('A', 15), 3, 'D'},

    { 5, GPIO('B', 16), 0, 'C'},
    { 5, GPIO('B', 17), 1, 'C'},
    { 5, GPIO('A', 20), 2, 'C'},
    { 5, GPIO('A', 21), 3, 'C'},
    { 5, GPIO('A', 22), 0, 'D'},
    { 5, GPIO('A', 23), 1, 'D'},
    { 5, GPIO('A', 24), 2, 'D'},
    { 5, GPIO('A', 25), 3, 'D'},
    { 5, GPIO('B', 30), 0, 'D'},
    { 5, GPIO('B', 31), 1, 'D'},
    { 5, GPIO('B', 22), 2, 'D'},
    { 5, GPIO('B', 23), 3, 'D'},
    { 5, GPIO('B', 2), 0, 'D'},
    { 5, GPIO('B', 3), 1, 'D'},
    { 5, GPIO('B', 0), 2, 'D'},
    { 5, GPIO('B', 1), 3, 'D'},
  #endif
#elif CONFIG_MACH_SAMX5
    { 0, GPIO('A', 8), 0, 'C'},
    { 0, GPIO('A', 9), 1, 'C'},
    { 0, GPIO('A', 10), 2, 'C'},
    { 0, GPIO('A', 11), 3, 'C'},
    { 0, GPIO('B', 24), 0, 'C'},
    { 0, GPIO('B', 25), 1, 'C'},
    { 0, GPIO('C', 24), 2, 'C'},
    { 0, GPIO('C', 25), 3, 'C'},
    { 0, GPIO('A', 4), 0, 'D'},
    { 0, GPIO('A', 5), 1, 'D'},
    { 0, GPIO('A', 6), 2, 'D'},
    { 0, GPIO('A', 7), 3, 'D'},
    { 0, GPIO('C', 17), 0, 'D'},
    { 0, GPIO('C', 16), 1, 'D'},
    { 0, GPIO('C', 18), 2, 'D'},
    { 0, GPIO('C', 19), 3, 'D'},

    { 1, GPIO('A', 16), 0, 'C'},
    { 1, GPIO('A', 17), 1, 'C'},
    { 1, GPIO('A', 18), 2, 'C'},
    { 1, GPIO('A', 19), 3, 'C'},
    { 1, GPIO('C', 22), 0, 'C'},
    { 1, GPIO('C', 23), 1, 'C'},
    { 1, GPIO('D', 20), 2, 'C'},
    { 1, GPIO('D', 21), 3, 'C'},
    { 1, GPIO('C', 27), 0, 'C'},
    { 1, GPIO('C', 28), 1, 'C'},
    { 1, GPIO('B', 22), 2, 'C'},
    { 1, GPIO('B', 23), 3, 'C'},
    { 1, GPIO('A', 0), 0, 'D'},
    { 1, GPIO('A', 1), 1, 'D'},
    { 1, GPIO('A', 30), 2, 'D'},
    { 1, GPIO('A', 31), 3, 'D'},

    { 2, GPIO('A', 12), 0, 'C'},
    { 2, GPIO('A', 13), 1, 'C'},
    { 2, GPIO('A', 14), 2, 'C'},
    { 2, GPIO('A', 15), 3, 'C'},
    { 2, GPIO('B', 26), 0, 'C'},
    { 2, GPIO('B', 27), 1, 'C'},
    { 2, GPIO('B', 28), 2, 'C'},
    { 2, GPIO('B', 29), 3, 'C'},
    { 2, GPIO('A', 9), 0, 'D'},
    { 2, GPIO('A', 8), 1, 'D'},
    { 2, GPIO('A', 10), 2, 'D'},
    { 2, GPIO('A', 11), 3, 'D'},
    { 2, GPIO('B', 25), 0, 'D'},
    { 2, GPIO('B', 24), 1, 'D'},
    { 2, GPIO('C', 24), 2, 'D'},
    { 2, GPIO('C', 25), 3, 'D'},

    { 3, GPIO('A', 22), 0, 'C'},
    { 3, GPIO('A', 23), 1, 'C'},
    { 3, GPIO('A', 24), 2, 'C'},
    { 3, GPIO('A', 25), 3, 'C'},
    { 3, GPIO('B', 20), 0, 'C'},
    { 3, GPIO('B', 21), 1, 'C'},
    { 3, GPIO('A', 20), 2, 'D'},
    { 3, GPIO('A', 21), 3, 'D'},
    { 3, GPIO('A', 17), 0, 'D'},
    { 3, GPIO('A', 16), 1, 'D'},
    { 3, GPIO('A', 18), 2, 'D'},
    { 3, GPIO('A', 19), 3, 'D'},
    { 3, GPIO('C', 23), 0, 'D'},
    { 3, GPIO('C', 22), 1, 'D'},
    { 3, GPIO('D', 20), 2, 'D'},
    { 3, GPIO('D', 21), 3, 'D'},

    { 4, GPIO('B', 12), 0, 'C'},
    { 4, GPIO('B', 13), 1, 'C'},
    { 4, GPIO('B', 14), 2, 'C'},
    { 4, GPIO('B', 15), 3, 'C'},
    { 4, GPIO('B', 8), 0, 'D'},
    { 4, GPIO('B', 9), 1, 'D'},
    { 4, GPIO('B', 10), 2, 'D'},
    { 4, GPIO('B', 11), 3, 'D'},
    { 4, GPIO('A', 13), 0, 'D'},
    { 4, GPIO('A', 12), 1, 'D'},
    { 4, GPIO('A', 14), 2, 'D'},
    { 4, GPIO('A', 15), 3, 'D'},
    { 4, GPIO('B', 27), 0, 'D'},
    { 4, GPIO('B', 26), 1, 'D'},
    { 4, GPIO('B', 28), 2, 'D'},
    { 4, GPIO('B', 29), 3, 'D'},

    { 5, GPIO('B', 16), 0, 'C'},
    { 5, GPIO('B', 17), 1, 'C'},
    { 5, GPIO('B', 18), 2, 'C'},
    { 5, GPIO('B', 19), 3, 'C'},
    { 5, GPIO('A', 23), 0, 'D'},
    { 5, GPIO('A', 22), 1, 'D'},
    { 5, GPIO('A', 20), 2, 'D'},
    { 5, GPIO('A', 21), 3, 'D'},
    { 5, GPIO('A', 24), 2, 'D'},
    { 5, GPIO('A', 25), 3, 'D'},
    { 5, GPIO('B', 22), 2, 'D'},
    { 5, GPIO('B', 23), 3, 'D'},
    { 5, GPIO('B', 31), 0, 'D'},
    { 5, GPIO('B', 30), 1, 'D'},
    { 5, GPIO('B', 0), 2, 'D'},
    { 5, GPIO('B', 1), 3, 'D'},
    { 5, GPIO('B', 2), 0, 'D'},
    { 5, GPIO('B', 3), 1, 'D'},
  #ifdef SERCOM6
    { 6, GPIO('C', 16), 0, 'C'},
    { 6, GPIO('C', 17), 1, 'C'},
    { 6, GPIO('C', 18), 2, 'C'},
    { 6, GPIO('C', 19), 3, 'C'},
    { 6, GPIO('C', 4), 0, 'C'},
    { 6, GPIO('C', 5), 1, 'C'},
    { 6, GPIO('C', 6), 2, 'C'},
    { 6, GPIO('C', 7), 3, 'C'},
    { 6, GPIO('D', 9), 0, 'D'},
    { 6, GPIO('D', 8), 1, 'D'},
    { 6, GPIO('D', 10), 2, 'D'},
    { 6, GPIO('D', 11), 3, 'D'},
    { 6, GPIO('C', 13), 0, 'D'},
    { 6, GPIO('C', 12), 1, 'D'},
    { 6, GPIO('C', 14), 2, 'D'},
    { 6, GPIO('C', 15), 3, 'D'},
    { 6, GPIO('C', 10), 2, 'C'},
    { 6, GPIO('C', 11), 3, 'C'},

    { 7, GPIO('C', 12), 0, 'C'},
    { 7, GPIO('C', 13), 1, 'C'},
    { 7, GPIO('C', 14), 2, 'C'},
    { 7, GPIO('C', 15), 3, 'C'},
    { 7, GPIO('D', 8), 0, 'C'},
    { 7, GPIO('D', 9), 1, 'C'},
    { 7, GPIO('D', 10), 2, 'C'},
    { 7, GPIO('D', 11), 3, 'C'},
    { 7, GPIO('C', 10), 2, 'D'},
    { 7, GPIO('C', 11), 3, 'D'},
    { 7, GPIO('B', 21), 0, 'D'},
    { 7, GPIO('B', 20), 1, 'D'},
    { 7, GPIO('B', 18), 2, 'D'},
    { 7, GPIO('B', 19), 3, 'D'},
    { 7, GPIO('B', 30), 0, 'C'},
    { 7, GPIO('B', 31), 1, 'C'},
    { 7, GPIO('A', 30), 2, 'C'},
    { 7, GPIO('A', 31), 3, 'C'},
  #endif
#endif
};

static const struct sercom_pad *
sercom_lookup_pad(uint32_t sercom_id, uint8_t pin)
{
    const struct sercom_pad *sp = sercom_pads;
    for (; ; sp++) {
        if (sp >= &sercom_pads[ARRAY_SIZE(sercom_pads)])
            shutdown("Invalid SERCOM configuration");
        if (sp->sercom_id == sercom_id && sp->gpio == pin)
            return sp;
    }
}


/****************************************************************
 * Runtime configuration
 ****************************************************************/

enum { TX_PIN, RX_PIN, CLK_PIN };
DECL_ENUMERATION("sercom_pin_type", "tx", TX_PIN);
DECL_ENUMERATION("sercom_pin_type", "rx", RX_PIN);
DECL_ENUMERATION("sercom_pin_type", "clk", CLK_PIN);

// Runtime configuration
struct sercom_pin {
    uint8_t pins[3];
};

static struct sercom_pin sercom_pins[ARRAY_SIZE(sercoms)];

void
command_set_sercom_pin(uint32_t *args)
{
    uint8_t sercom_id = args[0], pin_type = args[1], pin = args[2];
    if (sercom_id >= ARRAY_SIZE(sercom_pins)
        || pin_type >= ARRAY_SIZE(sercom_pins[0].pins))
        shutdown("Invalid SERCOM bus");
    sercom_pins[sercom_id].pins[pin_type] = pin;
}
DECL_COMMAND(command_set_sercom_pin,
             "set_sercom_pin bus=%u sercom_pin_type=%u pin=%u");

void
nothing(void)
{

}

void (*SERCOM_App_Handlers[CONFIG_SERCOMS])(uint32_t sercom_id) = { NULL }; 

void
SERCOM0_HANDLER(void) {
    if (SERCOM_App_Handlers[0] != NULL)
        (*SERCOM_App_Handlers[0])(0);
}

void
SERCOM1_HANDLER(void)
{
    if (SERCOM_App_Handlers[1] != NULL)
        (*SERCOM_App_Handlers[1])(1);
}

void
SERCOM2_HANDLER(void)
{
    if (SERCOM_App_Handlers[2] != NULL)
        (*SERCOM_App_Handlers[2])(2);
}

void
SERCOM3_HANDLER(void)
{
    if (SERCOM_App_Handlers[3] != NULL)
        (*SERCOM_App_Handlers[3])(3);
}

#ifdef SERCOM4
void
SERCOM4_HANDLER(void)
{
    if (SERCOM_App_Handlers[4] != NULL)
        (*SERCOM_App_Handlers[4])(4);
}

void
SERCOM5_HANDLER(void)
{
    if (SERCOM_App_Handlers[5] != NULL)
        (*SERCOM_App_Handlers[5])(5);
}
#endif
#ifdef SERCOM6
void
SERCOM6_HANDLER(void)
{
    if (SERCOM_App_Handlers[6] != NULL)
        (*SERCOM_App_Handlers[6])(6);
}

void
SERCOM7_HANDLER(void)
{
    if (SERCOM_App_Handlers[7] != NULL)
        (*SERCOM_App_Handlers[7])(7);
}
#endif

#if CONFIG_MACH_SAMD21
    #define sercom_set_irq_handler(sercom_id) \
        armcm_enable_irq(SERCOM##sercom_id##_HANDLER, SERCOM##sercom_id##_IRQn, 0);
#elif CONFIG_MACH_SAMD51 || CONFIG_MACH_SAME51
    #define sercom_set_irq_handler(sercom_id) \
        armcm_enable_irq(SERCOM##sercom_id##_HANDLER, SERCOM##sercom_id##_0_IRQn, 0); \
        armcm_enable_irq(SERCOM##sercom_id##_HANDLER, SERCOM##sercom_id##_1_IRQn, 0); \
        armcm_enable_irq(SERCOM##sercom_id##_HANDLER, SERCOM##sercom_id##_2_IRQn, 0); \
        armcm_enable_irq(SERCOM##sercom_id##_HANDLER, SERCOM##sercom_id##_3_IRQn, 0);
#endif

void sercom_irq_init(void) {
    // Enable handers for all channels except the one that is used for serial
    // communication to the host controller
    #if !CONFIG_ATSAMD_SERIAL_SERCOM0
        sercom_set_irq_handler(0);
    #endif
    #if !CONFIG_ATSAMD_SERIAL_SERCOM1
        sercom_set_irq_handler(1);
    #endif
    #if !CONFIG_ATSAMD_SERIAL_SERCOM2
        sercom_set_irq_handler(2);
    #endif
    #if !CONFIG_ATSAMD_SERIAL_SERCOM3
        sercom_set_irq_handler(3);
    #endif
    #ifdef SERCOM4
        #if !CONFIG_ATSAMD_SERIAL_SERCOM4
            sercom_set_irq_handler(4);
        #endif
        #if !CONFIG_ATSAMD_SERIAL_SERCOM5
            sercom_set_irq_handler(5);
        #endif
    #endif
    #ifdef SERCOM6
        #if !CONFIG_ATSAMD_SERIAL_SERCOM6
            sercom_set_irq_handler(6);
        #endif
        #if !CONFIG_ATSAMD_SERIAL_SERCOM7
            sercom_set_irq_handler(7);
        #endif
    #endif
}


void
set_sercom_interrupt(uint32_t sercom_id, void (*handler)(uint32_t sercom_id))
{
    // Switch statement works around need for the interrupt number in armcm_enable_irq to be constant
    if (sercom_id < CONFIG_SERCOMS) {
        SERCOM_App_Handlers[sercom_id] = handler;
    }
}

void
clear_sercom_interrupt(uint32_t sercom_id)
{
    set_sercom_interrupt(sercom_id, NULL);
}


/****************************************************************
 * SPI dopo flag mapping
 ****************************************************************/

struct sercom_spi_map {
    uint8_t tx_pad, clk_pad, dopo;
};

static const struct sercom_spi_map sercom_spi[] = {
    { 0, 1, 0 },
    { 3, 1, 2 },
#if CONFIG_MACH_SAMD21
    { 2, 3, 1 },
    { 0, 3, 3 },
#endif
};

static uint8_t
sercom_lookup_spi_dopo(uint8_t tx_pad, uint8_t clk_pad)
{
    const struct sercom_spi_map *sm = sercom_spi;
    for (; ; sm++) {
        if (sm >= &sercom_spi[ARRAY_SIZE(sercom_spi)])
            shutdown("Invalid combination of TX pin and CLK pin");
        if (sm->tx_pad == tx_pad && sm->clk_pad == clk_pad)
            return sm->dopo;
    }
}


/****************************************************************
 * Pin setup
 ****************************************************************/

uint32_t
sercom_serial_pins(uint32_t sercom_id, uint8_t tx_pin, uint8_t rx_pin) {
    const struct sercom_pad *tx_sp = sercom_lookup_pad(sercom_id, tx_pin);
    const struct sercom_pad *rx_sp = sercom_lookup_pad(sercom_id, rx_pin);

    gpio_peripheral(tx_pin, tx_sp->ptype, 0);
    gpio_peripheral(rx_pin, rx_sp->ptype, 0);

    return SERCOM_USART_CTRLA_RXPO(rx_sp->pad) | SERCOM_USART_CTRLA_TXPO(0);
}

uint32_t
sercom_usart_pins(uint32_t sercom_id, uint8_t mode)
{
    uint8_t tx_pin = sercom_pins[sercom_id].pins[TX_PIN];
    const struct sercom_pad *tx_sp = sercom_lookup_pad(sercom_id, tx_pin);
    uint8_t rx_pin = sercom_pins[sercom_id].pins[RX_PIN];
    const struct sercom_pad *rx_sp = sercom_lookup_pad(sercom_id, rx_pin);
    uint8_t clk_pin;
    const struct sercom_pad *clk_sp;
    if (mode & UF_HAVE_XCK) {
      clk_pin = sercom_pins[sercom_id].pins[CLK_PIN];
      clk_sp = sercom_lookup_pad(sercom_id, clk_pin);
    }

    // TX is 0
    // CLK is 1 or none
    // RX is any remaining

    if (tx_sp->pad != 0 || rx_sp->pad == 0)
        shutdown("TX pin not on PAD0 or RX pin collides with TX pin");
    if (mode & UF_HAVE_XCK && (clk_sp->pad != 1 || rx_sp->pad == 1))
        shutdown("CLK pin not on PAD1 or RX pin collides with CLK pin");

    gpio_peripheral(tx_pin, tx_sp->ptype, 0);
    gpio_peripheral(rx_pin, rx_sp->ptype, 0);
    if (mode & UF_HAVE_XCK) {
      gpio_peripheral(clk_pin, clk_sp->ptype, 0);
    }
    return SERCOM_USART_CTRLA_RXPO(rx_sp->pad) | SERCOM_USART_CTRLA_TXPO(0);
}

// TODO: Add definition for uart/usart pins

uint32_t
sercom_spi_pins(uint32_t sercom_id)
{
    if (sercom_id >= ARRAY_SIZE(sercom_pins))
        shutdown("Invalid SERCOM bus");
    uint8_t tx_pin = sercom_pins[sercom_id].pins[TX_PIN];
    const struct sercom_pad *tx_sp = sercom_lookup_pad(sercom_id, tx_pin);
    uint8_t rx_pin = sercom_pins[sercom_id].pins[RX_PIN];
    const struct sercom_pad *rx_sp = sercom_lookup_pad(sercom_id, rx_pin);
    uint8_t clk_pin = sercom_pins[sercom_id].pins[CLK_PIN];
    const struct sercom_pad *clk_sp = sercom_lookup_pad(sercom_id, clk_pin);

    uint8_t dopo = sercom_lookup_spi_dopo(tx_sp->pad, clk_sp->pad);
    if (rx_sp->pad == tx_sp->pad || rx_sp->pad == clk_sp->pad)
        shutdown("Sercom RX pad collides with TX or CLK pad");

    gpio_peripheral(tx_pin, tx_sp->ptype, 0);
    gpio_peripheral(rx_pin, rx_sp->ptype, 0);
    gpio_peripheral(clk_pin, clk_sp->ptype, 0);
    return SERCOM_SPI_CTRLA_DIPO(rx_sp->pad) | SERCOM_SPI_CTRLA_DOPO(dopo);
}

void
sercom_i2c_pins(uint32_t sercom_id)
{
    uint8_t tx_pin = sercom_pins[sercom_id].pins[TX_PIN];
    const struct sercom_pad *tx_sp = sercom_lookup_pad(sercom_id, tx_pin);
    uint8_t clk_pin = sercom_pins[sercom_id].pins[CLK_PIN];
    const struct sercom_pad *clk_sp = sercom_lookup_pad(sercom_id, clk_pin);

    if (tx_sp->pad != 0 || clk_sp->pad != 1)
        shutdown("TX pin not on PAD0 or CLK pin not on PAD1");

    gpio_peripheral(tx_pin, tx_sp->ptype, 0);
    gpio_peripheral(clk_pin, clk_sp->ptype, 0);
}

DECL_INIT(sercom_irq_init);
