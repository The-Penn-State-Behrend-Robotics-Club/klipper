// uart support on samd
//
// Copyright (C) 2022 Alan Everett <thatcomputerguy0101@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdlib.h>
#include "internal.h" // enable_pclock
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

    // CTRLB Stuff:
    // LINCMD (LIN stuff)
    // RXEN rx enable is 0 for disabled, 1 for enabled; default disabled
    // TXEN tx enable is 0 for disabled, 1 for enabled; default disabled
    // PMODE parity mode is 0 for even parity bit, 1 for odd parity bit
    // ENC encoding format is 0 for no encoding, 1 for IrDA encoding
    // SFDE start of frame detection enable is 0 for disabled, 1 for enabled
    // COLDEN collision detection enable is 0 for disabled, 1 for enabled
    // SBMODE stop bit mode is 0 for one tx stop bit, 1 for two tx stop bits
    // CHSIZE chacter size; default is 8 bits, can be set 5-9

    // CTRLC Stuff:
    // DATA32B data register RW control; 0 is CHSIZE for both, low bit controls write, high bit controls read
    // MAXITER (ISO7816 T=0 mode stuff)
    // DSNACK (ISO7816 T=0 mode stuff)
    // INACK (ISO7816 T=0 mode stuff)
    // HDRDLY (LIN stuff)
    // BRKLEN (LIN stuff)
    // GTIME guard time in RS485 mode

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

struct usart_buffer* usart_buffers[CONFIG_SERCOMS] = { NULL }; 

void
usart_buffer_handler(uint32_t sercom_id) {
    SercomUsart *su = &sercom_get_by_id(sercom_id)->USART;
    uint32_t status = su->INTFLAG.reg;
    struct usart_buffer* buffer = usart_buffers[sercom_id];
    if (status & SERCOM_USART_INTFLAG_RXC) {
        // Handle recieved bytess
        while (buffer->rx.len >= buffer->rx.maxlen) {} // Wait for room
        int32_t buffer_end = (buffer->rx.start + (buffer->rx.len ++)) % buffer->rx.maxlen;
        buffer->rx.data[buffer_end] = su->DATA.reg;
    }
    if (status & SERCOM_USART_INTFLAG_DRE) {
        // TX Data register has been emptied
        if (buffer->tx.len > 0) {
            // Data available to transmit
            su->DATA.reg = buffer->tx.data[buffer->tx.start ++];
            buffer->tx.start %= buffer->tx.maxlen;
            buffer->tx.len --;
        } else {
            // Data depleted
            // Disable further TX interrupts
            su->INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
        }
    }
}

struct usart_config
usart_setup(uint32_t bus, uint8_t mode, uint32_t rate, struct usart_buffer* buffer)
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
    if (buffer->rx.maxlen > 0) {
        su->INTENSET.reg = SERCOM_USART_INTENSET_RXC;
    }
    return (struct usart_config){ .su = su, .baud = baud, .buffer = buffer};
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
usart_write_unbuffered(struct usart_config config, uint8_t write_len, uint8_t *write)
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

void
usart_set_tx_buffer(struct usart_config config, uint8_t* buffer_data, uint8_t buffer_len)
{
    // Enables asynchronous reading with a buffer for the handler to put data directly into
    // Erases the oldest data upon being overflowed
    // Set buffer_len to 0 to disable

    if (buffer_len > 0) {
        config.buffer->tx.data = buffer_data;
        config.buffer->tx.maxlen = buffer_len;
        config.buffer->tx.start = 0;
        config.buffer->tx.len = 0;
        // Replace irq with dma?
    } else {
        config.buffer->tx.maxlen = 0;
    }
}

void
usart_set_rx_buffer(struct usart_config config, uint8_t* buffer_data, uint8_t buffer_len)
{
    // Enables asynchronous reading with a buffer for the handler to put data directly into
    // Erases the oldest data upon being overflowed
    // Set buffer_len to 0 to disable

    SercomUsart *su = (SercomUsart *)config.su;

    if (buffer_len > 0) {
        config.buffer->rx.data = buffer_data;
        config.buffer->rx.maxlen = buffer_len;
        config.buffer->rx.start = 0;
        config.buffer->rx.len = 0;
        // Enable RX interrupts
        su->INTENSET.reg = SERCOM_USART_INTENSET_RXC;
        // Replace irq with dma?
    } else {
        config.buffer->rx.maxlen = 0;
        // Disable RX interrupts
        su->INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;
    }
}

void
usart_write(struct usart_config config, uint8_t write_len, uint8_t *write)
{
    if (config.buffer->tx.maxlen == 0) {
        return usart_write_unbuffered(config, write_len, write);
    }

    // Write data to buffer until full
    while (write_len--) {
        while (config.buffer->tx.len >= config.buffer->tx.maxlen); // Pause if the buffer is full for some data to be transmitted
        uint32_t buffer_end = (config.buffer->tx.start + (config.buffer->tx.len ++)) % config.buffer->tx.maxlen;
        config.buffer->tx.data[buffer_end] = *write++;
    }

    SercomUsart *su = (SercomUsart *)config.su;

    if (config.buffer->tx.len > 0) {
        // Data available to transmit
        su->INTENSET.reg = SERCOM_USART_INTENSET_DRE;

        su->DATA.reg = config.buffer->tx.data[config.buffer->tx.start ++];
        config.buffer->tx.start %= config.buffer->tx.maxlen;
        config.buffer->tx.len --;
    }
}

void
usart_read(struct usart_config config, uint8_t read_len, uint8_t *read)
{
    if (config.buffer->rx.maxlen == 0) {
        return usart_read_unbuffered(config, read_len, read);
    }
    // Read buffered data
    for (int i = 0; i < read_len; i++) {
        while (config.buffer->rx.len > 0); // Pause if the buffer is empty for more data to be recieved
        // for (int j = 0; j < 4 && i < read_len; j++, i++) {
        //     // Splitting 32 bit data into 8 bit data
        //     *read++ = (uint8_t)(*(config.buffer.data + config.buffer.start) >> (j * 4) & 0xff);
        // }
        *read++ = *(config.buffer->rx.data + config.buffer->rx.start);
        config.buffer->rx.start++;
        config.buffer->rx.start %= config.buffer->rx.maxlen;
        config.buffer->rx.len--;
    }
}
