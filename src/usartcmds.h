#ifndef __USARTCMDS_H
#define __USARTCMDS_H

#include <inttypes.h>
#include "board/gpio.h" // usart_config

struct usartdev_s {
    struct usart_config usart_config;
    uint8_t flags;
};

enum {
    UF_HAVE_XCK = 1
};

struct usartdev_s *usartdev_oid_lookup(uint8_t oid);

#endif
