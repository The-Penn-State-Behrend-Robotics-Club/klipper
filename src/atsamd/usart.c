// uart support on samd
//
// Copyright (C) 2022 Alan Everett <thatcomputerguy0101@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdlib.h>
#include "internal.h" // enalbe_pclock
#include "gpio.h" // uart_setup
#include "command.h" // output

// TODO: Implement a buffer for asynchronous reading

void
usart_init(uint32_t bus, SercomUsart *su, uint32_t baud, uint32_t tx_rx_clk)
{
    static uint8_t have_run_init;
    if (have_run_init & (1<<bus))
        return;
    have_run_init |= 1<<bus;

    // CTRLA Stuff:
    // DORD data order
    // CPOL clock polarity
    // CMODE clock mode is 0 for uart, 1 for usart
    // FORM frame format is 0 for usart, 1 for usart + parity, 2 for LIN master, 4 for LIN slave, 5 for LIN slave + parity, 7 for smart cards
    // SAMPA sampling adjustment
    // RXPO recieve pin (SERCOM pad index)
    // TXPO transmit pins is 0 for USART or UART (tx, xck), 2 for UART (tx, rts/te), cts, 3 for RS485 (tx, te)
    // SAMPR sampling rate & fractional sampling
    // RXINV rx invert is 0 for not inverted, 1 for inverted
    // TXINV tx invert is 0 for not inverted, 1 for inverted
    // IBON immediate buffer overflow notification (Default)
    // RUNSTDBY run in standby (Default)
    // MODE SERCOM mode is 0 for external clock source (sync client), 1 for internal clock source (sync host or async)

    // Configure usart
    su->CTRLA.reg = 0;
    uint32_t ctrla = (SERCOM_USART_CTRLA_MODE(1)
                    | SERCOM_USART_CTRLA_DORD
                    | SERCOM_USART_CTRLA_SAMPR(1)
                    | tx_rx_clk);
    su->CTRLA.reg = ctrla;
    su->CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
    su->BAUD.reg = (SERCOM_USART_BAUD_FRAC_BAUD(baud / 8)
                    | SERCOM_USART_BAUD_FRAC_FP(baud % 8));
    // Enable Sercom
    su->CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
}

struct sercom_buffer* usart_buffers[CONFIG_SERCOMS] = { NULL }; 

void
usart_buffer_handler(uint32_t sercom_id) {

}

struct usart_config
usart_setup(uint32_t bus, uint8_t mode, uint32_t rate, struct sercom_buffer* buffer)
{
    uint32_t tx_rx_clk = sercom_usart_pins(bus, mode);
    // Enable serial clock
    Sercom *sercom = sercom_enable_pclock(bus);
    SercomUsart *su = &sercom->USART;
    uint32_t baud = sercom_get_pclock_frequency(bus) / (2 * rate);
    usart_init(bus, su, baud, tx_rx_clk);
    // Enable IRQs
    set_sercom_interrupt(bus, usart_buffer_handler);
    usart_buffers[bus] = buffer;
    return (struct usart_config){ .su = su, .baud = baud};
}

/*
static void
usart_wait(SercomUsart *su)
{
    for (;;) {
        uint32_t intflag = su->INTFLAG.reg;
        if (intflag & SERCOM_USART_INTFLAG_RXC)
            // Byte recieved
            usart_rx_byte(su->DATA.reg);
        if (intflag & SERCOM_USART_INTFLAG_DRE) {
            // No more bytes to transmit
            uint8_t data;
            int ret = usart_get_tx_byte(&data);
            if (ret)
                su->INTENCLR.reg = SERCOM_USART_INTENSET_DRE;
            else
                su->DATA.reg = data;
        }
        #error "This is still a handler, not a loop"
    }
}
*/

void
usart_write(struct usart_config config, uint8_t write_len, uint8_t *write)
{
    SercomUsart *su = (SercomUsart *)config.su;
    while (write_len--) {
        su->DATA.reg = *write++;
        while (!(su->INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
    }

}

void
usart_read_unbuffered(struct usart_config config, uint8_t read_len, uint8_t *read)
{
    SercomUsart *su = (SercomUsart *)config.su;

    for (int i = 0; i < read_len; i++) {
        while (!(su->INTFLAG.reg & SERCOM_USART_INTFLAG_RXC));
        // for (int j = 0; j < 4 && i < read_len; j++, i++) {
        //     // Splitting 32 bit data into 8 bit data
        //     *read++ = (uint8_t)(su->DATA.reg >> (j * 4) & 0xff);
        // }
        *read++ = su->DATA.reg;
    }
}
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

void
usart_set_buffer(struct usart_config config, struct sercom_buffer* buffer, uint32_t* buffer_data, uint8_t buffer_len)
{
    // Enables asynchronous reading with a buffer for the handler to put data directly into
    // Erases the oldest data upon being overflowed
    // Set buffer_len to 0 to disable

    SercomUsart *su = (SercomUsart *)config.su;

    if (buffer_len > 0) {
        buffer->data = buffer_data;
        buffer->len = buffer_len;
        buffer->start = 0;
        buffer->end = 0;
        // Enable irqs
        su->INTENSET.reg = SERCOM_USART_INTENSET_RXC;
        // Replace irq with dma?
    } else {
        buffer->len = 0;
        su->INTENCLR.reg = SERCOM_USART_INTENSET_RXC;
    }
}

void
usart_read(struct usart_config config, uint8_t read_len, uint8_t *read)
{
    SercomUsart *su = (SercomUsart *)config.su;
    int i = 0;

    // Read buffered data
    for (; i < read_len && config.buffer.start != config.buffer.end; i++) {
        // for (int j = 0; j < 4 && i < read_len; j++, i++) {
        //     // Splitting 32 bit data into 8 bit data
        //     *read++ = (uint8_t)(*(config.buffer.data + config.buffer.start) >> (j * 4) & 0xff);
        // }
        *read++ = *(config.buffer.data + config.buffer.start);
        config.buffer.start++;
        config.buffer.start %= config.buffer.len;
    }

    // If buffer runs out, wait for more data to arrive
    for (; i < read_len; i++) {
        while (!(su->INTFLAG.reg & SERCOM_USART_INTFLAG_RXC));
        // for (int j = 0; j < 4 && i < read_len; j++, i++) {
        //     // Splitting 32 bit data into 8 bit data
        //     *read++ = (uint8_t)(su->DATA.reg >> (j * 4) & 0xff);
        // }
        *read++ = su->DATA.reg;
        // TODO: Debug here to figure out what is happening between data sizes
    }
}
