// Commands for sending messages on a usart bus
//
// Copyright (C) 2018  Florian Heilmann <Florian.Heilmann@gmx.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memcpy
#include "basecmd.h" //oid_alloc
#include "command.h"  //sendf
#include "sched.h" //DECL_COMMAND
#include "board/gpio.h" //usart_write/read/setup
#include "usartcmds.h"

void
command_config_usart(uint32_t *args)
{
    struct usartdev_s *usart = oid_alloc(args[0], command_config_usart, sizeof(*usart));
    usart->usart_config = usart_setup(args[1], UF_HAVE_XCK, args[2], &usart->buffer);
    usart->flags |= UF_HAVE_XCK;
}
DECL_COMMAND(command_config_usart,
             "config_usart oid=%c usart_bus=%u rate=%u");

void
command_config_uart(uint32_t *args)
{
   struct usartdev_s *usart = oid_alloc(args[0], command_config_usart, sizeof(*usart));
   usart->usart_config = usart_setup(args[1], 0x0, args[2], &usart->buffer);
}
DECL_COMMAND(command_config_uart,
            "config_uart oid=%c uart_bus=%u rate=%u");

struct usartdev_s *
usartdev_oid_lookup(uint8_t oid)
{
    return oid_lookup(oid, command_config_usart);
}

void
command_usart_write(uint32_t *args)
{
    uint8_t oid = args[0];
    struct usartdev_s *usart = oid_lookup(oid, command_config_usart);
    uint8_t data_len = args[1];
    uint8_t *data = command_decode_ptr(args[2]);
    usart_write(usart->usart_config, data_len, data);
}
DECL_COMMAND(command_usart_write, "usart_write oid=%c data=%*s");

void
command_usart_read(uint32_t *args)
{
    uint8_t oid = args[0];
    struct usartdev_s *usart = oid_lookup(oid, command_config_usart);
    uint8_t data_len = args[1];
    uint8_t data[data_len];
    usart_read(usart->usart_config, data_len, data);
    sendf("usart_read_response oid=%c response=%*s", oid, data_len, data);
}
DECL_COMMAND(command_usart_read, "usart_read oid=%c read_len=%u");

void
command_usart_transfer(uint32_t *args)
{
    uint8_t oid = args[0];
    struct usartdev_s *usart = oid_lookup(oid, command_config_usart);
    uint8_t write_len = args[1];
    uint8_t *write_data = command_decode_ptr(args[2]);
    uint8_t read_len = args[3];
    uint8_t buffer[read_len];
    usart_set_rx_buffer(usart->usart_config, buffer, read_len);
    usart_write(usart->usart_config, write_len, write_data);
    uint8_t read_data[read_len];
    usart_read(usart->usart_config, read_len, read_data);
    sendf("usart_read_response oid=%c response=%*s", oid, read_len, read_data);
}
DECL_COMMAND(command_usart_transfer, "usart_transfer oid=%c data=%*s read_len=%u");

/* TODO: Evaluate whether these unbuffered commands are actually useful
   (I think not, since the regular ones will default to no buffer if
   there isn't one set) */
void
command_usart_write_unbuffered(uint32_t *args)
{
    uint8_t oid = args[0];
    struct usartdev_s *usart = oid_lookup(oid, command_config_usart);
    uint8_t data_len = args[1];
    uint8_t *data = command_decode_ptr(args[2]);
    usart_write_unbuffered(usart->usart_config, data_len, data);
}
DECL_COMMAND(command_usart_write_unbuffered, "usart_write_unbuffered oid=%c data=%*s");

void
command_usart_read_unbuffered(uint32_t * args)
{
    uint8_t oid = args[0];
    struct usartdev_s *usart = oid_lookup(oid, command_config_usart);
    uint8_t data_len = args[1];
    uint8_t data[data_len];
    usart_read_unbuffered(usart->usart_config, data_len, data);
    sendf("usart_read_response oid=%c response=%*s", oid, data_len, data);
}
DECL_COMMAND(command_usart_read_unbuffered, "usart_read_unbuffered oid=%c read_len=%u");
