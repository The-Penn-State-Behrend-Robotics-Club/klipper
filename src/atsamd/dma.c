#include <stdlib.h>
#include "internal.h"

#define DMA_POOL_SIZE 16

static DmacDescriptor dmapool[DMA_POOL_SIZE] __attribute__((aligned(16))) = { NULL };

void dma_init(void) {
    static uint8_t have_run_init = 0; 
    if (have_run_init)
        return;
    have_run_init = 1;

    DMAC->CTRL.reg = 0;
    while (DMAC->CTRL.bit.DMAENABLE); // Ensure DMA is disabled before editing it
    DMAC->CTRL.bit.SWRST = 1;
    while (DMAC->CTRL.bit.SWRST); // Wait for software reset to complete

    // Provide pointers for DMAC control interaction
    DMAC->BASEADDR.reg = &dmadescriptor;
    DMAC->WRBADDR.reg = &dmawriteback;

    // Set priority levels if more than 1 DMA channel is used
}

// Put these in some header file
#define DCF_NONE 0
#define DCF_CRC16_8 1

void dma_setup(uint8_t channel, uint32_t source, uint8_t crc_mode) {
    dma_init();
    // TODO, identify TRIGACT setting, TRIGSRC datatype, etc...
    DMAC->Channel[channel].CHCTRLA.reg = DMAC_CHCTRLA_TRIGACT_BURST | DMAC_CHCTRLA_TRIGSRC(source);
    // TODO, more DMA stuff...

    // CRC setup (Page 379)
    if (crc_mode == DCF_CRC16_8) {
        DMAC->CRCCTRL.reg = (0x20 | channel) << DMAC_CRCCTRL_CRCSRC_Pos
                          | DMAC_CRCCTRL_CRCPOLY_CRC16
                          | DMAC_CRCCTRL_CRCBEATSIZE(8);
    }
}

void dma_request(void) {
    // 8 bit beat transfers
    // validity?
    dmadescriptor.BTCTRL.reg = DMAC_BTCTRL_BEATSIZE(8) | DMAC_BTCTRL_VALID;

    dmadescriptor.BTCNT.reg = DMAC_BTCNT_BTCNT(?); // Page 443

}

void DMAC_0_Handler(void) {

}
