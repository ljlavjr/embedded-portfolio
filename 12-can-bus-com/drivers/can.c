#include "can.h"
#include "gpio.h"
#include "stm32f407.h"

typedef struct {
    uint16_t brp;
    uint8_t tseg1;
    uint8_t tseg2;
    uint8_t sjw;
} can_timing_t;

static const can_timing_t bitrate_table[] = {
    [CAN_BITRATE_125K] = { 23, 10, 1, 0 },  // PLACEHOLDER, calculate later
    [CAN_BITRATE_250K] = { 11, 10, 1, 0 },  // PLACEHOLDER, calculate later
    [CAN_BITRATE_500K] = {  5, 10, 1, 0 },  // PLACEHOLDER, calculate later
    [CAN_BITRATE_1M]   = {  2, 10, 1, 0 },  // PLACEHOLDER, calculate later
};

#define CAN_INIT_TIMEOUT    1000000

// Request init mode. Waits for hardware to acknowledge by setting INAK in MSR.
// Returns 0 on success, -1 on timeout
static int can_enter_init_mode(void) {
    // Set INRQ (bit 0) in MCR to request init mode
    // Clear SLEEP (bit 1) in MCR in case peripheral was asleep
    CAN1->MCR |= (1 << 0);  // INRQ = 1
    CAN1->MCR &= ~(1 << 1); // SLEEP = 0

    // Wait for INAK (bit 0 in MSR) to confirm init mode
    uint32_t timeout = CAN_INIT_TIMEOUT;
    while (!(CAN1->MSR & (1 << 0))) {
        if (--timeout == 0) return -1;
    }

    return 0;
}

// Request normal mode. Waits for INAK to clear
// Returns 0 on success, -1 on timeout
static int can_leave_init_mode(void) {
    // Clear INRQ to leave in it mode
    CAN1->MCR &= ~ (1 << 0);

    // Wait for INAK to clear
    uint32_t timeout = CAN_INIT_TIMEOUT;
    while (!(CAN1->MSR & (1 << 0))) {
        if (--timeout == 0) return -1;
    }
    
    return 0;
}

int can_init(can_bitrate_t bitrate, can_mode_t mode) {
    // Step 1: Enable CAN1 peripheral clock
    // CAN1 is on APB1, enable bit is bit 25 of RCC_APB1ENR
    RCC->APB1ENR |= (1 << 25);

    // Step 2: Configure GPIO pins for CAN1
    // PB8 = CAN1_RX, PB9 = CAN1_TX, both alternate function 9
    gpio_init(GPIOB, 8, GPIO_MODE_ALT);
    gpio_init(GPIOB, 9, GPIO_MODE_ALT);
    gpio_set_alt_func(GPIOB, 8, 9);   // PB8 = CAN1_RX, AF9
    gpio_set_alt_func(GPIOB, 9, 9);   // PB9 = CAN1_TX, AF9

    // Step 3: Enter init mode
    if (can_enter_init_mode() != 0) return -1;

    // Step 4: Configure MCR
    // For now use sensible defaults. Bits we care about:
    // ABOM (bit 6) = automatic bus off recovery (set to 1)
    // AWUM (bit 5) = automatic wake up (leave 0)
    // NART (bit 4) = no automatic retransmission (leave 0 - retries enabled)
    // RFLM (bit 3) = reveive FIFO locked (leave 0 - newest overwrites oldest)
    // TXFP (bit 2) = transmit FIFO priority by request order (leave 0 = by ID)
    CAN1->MCR |= (1 << 6);  // ABOM = 1, automatic bus off recovery

    // Step 5: Configure BTR
    // Build the BTR register value from the bit timing table plus mode bits
    const can_timing_t *t = &bitrate_table[bitrate];
    uint32_t btr = 0;
    btr |= ((uint32_t)t->brp   & 0x3FF) << 0;    // BRP at bits [9:0]
    btr |= ((uint32_t)t->tseg1 & 0xF)   << 16;   // TS1 at bits [19:16]
    btr |= ((uint32_t)t->tseg2 & 0x7)   << 20;   // TS2 at bits [22:20]
    btr |= ((uint32_t)t->sjw   & 0x3)   << 24;   // SJW at bits [25:24]

     // Apply mode flags to BTR.
    if (mode == CAN_MODE_LOOPBACK || mode == CAN_MODE_LOOPBACK_SILENT) {
        btr |= (1 << 30);    // LBKM = loopback
    }
    if (mode == CAN_MODE_SILENT || mode == CAN_MODE_LOOPBACK_SILENT) {
        btr |= (1U << 31);   // SILM = silent
    }

    CAN1->BTR = btr;

    // Step 6: Leave init mode and enter normal mode.
    if (can_leave_init_mode() != 0) return -2;

    return 0;
}

int can_filter_accept_all(void) {
    // Step 1: Enter filter init mode.
    // FINIT is bit 0 of CAN_FMR.
    CAN1->FMR |= (1 << 0);

    // Step 2: Deactivate filter bank 0 before configuring.
    // FACT0 is bit 0 of CAN_FA1R.
    CAN1->FA1R &= ~(1 << 0);

    // Step 3: Configure filter bank 0.

    // Set scale to 32 bit (single filter / mask pair).
    // FSC0 is bit 0 of CAN_FS1R. 1 = 32 bit, 0 = 16 bit.
    CAN1->FS1R |= (1 << 0);

    // Set mode to mask mode.
    // FBM0 is bit 0 of CAN_FM1R. 0 = mask mode, 1 = list mode.
    CAN1->FM1R &= ~(1 << 0);

    // Assign filter to FIFO 0.
    // FFA0 is bit 0 of CAN_FFA1R. 0 = FIFO 0, 1 = FIFO 1.
    CAN1->FFA1R &= ~(1 << 0);

    // Set ID = 0 and mask = 0.
    // Mask of 0 means "do not care about any bit", so all IDs match.
    CAN1->FR[0].FR1 = 0x00000000;   // ID
    CAN1->FR[0].FR2 = 0x00000000;   // Mask

    // Step 4: Activate filter bank 0.
    CAN1->FA1R |= (1 << 0);

    // Step 5: Leave filter init mode.
    CAN1->FMR &= ~(1 << 0);

    return 0;
}

int can_transmit(const can_frame_t *frame) {
    // Find an empty transmit mailbox.
    // TSR (transmit status register) bits 26, 27, 28 are TME0, TME1, TME2
    // (Transmit Mailbox Empty). Bits 24:25 are CODE, the next mailbox to be used.
    uint32_t timeout = CAN_INIT_TIMEOUT;
    int mailbox = -1;
    while (timeout--) {
        if (CAN1->TSR & (1 << 26))      { mailbox = 0; break; }
        else if (CAN1->TSR & (1 << 27)) { mailbox = 1; break; }
        else if (CAN1->TSR & (1 << 28)) { mailbox = 2; break; }
    }
    if (mailbox < 0) return -1;

    CAN_TX_Mailbox_TypeDef *mb = &CAN1->TX[mailbox];

    // Build TIR register: ID, IDE bit, RTR bit, do not set TXRQ yet.
    uint32_t tir = 0;
    if (frame->ide) {
        // 29 bit extended ID: goes in bits [31:3]. IDE = bit 2.
        tir |= (frame->id & 0x1FFFFFFF) << 3;
        tir |= (1 << 2);
    } else {
        // 11 bit standard ID: goes in bits [31:21]. IDE = 0.
        tir |= (frame->id & 0x7FF) << 21;
    }
    if (frame->rtr) {
        tir |= (1 << 1);    // RTR bit
    }
    mb->TIR = tir;

    // Set DLC in TDTR. DLC is bits [3:0].
    mb->TDTR = (frame->dlc & 0xF);

    // Pack data into TDLR (bytes 0 to 3) and TDHR (bytes 4 to 7).
    mb->TDLR = ((uint32_t)frame->data[0] << 0)  |
               ((uint32_t)frame->data[1] << 8)  |
               ((uint32_t)frame->data[2] << 16) |
               ((uint32_t)frame->data[3] << 24);
    mb->TDHR = ((uint32_t)frame->data[4] << 0)  |
               ((uint32_t)frame->data[5] << 8)  |
               ((uint32_t)frame->data[6] << 16) |
               ((uint32_t)frame->data[7] << 24);

    // Request transmission by setting TXRQ (bit 0 of TIR).
    mb->TIR |= (1 << 0);

    return 0;
}

int can_receive_available(void) {
    // RF0R bits [1:0] are FMP0 (FIFO 0 message pending count).
    // Non zero means at least one frame is waiting.
    return (CAN1->RF0R & 0x3) != 0;
}

int can_receive(can_frame_t *frame) {
    // Check that something is actually in the FIFO.
    if ((CAN1->RF0R & 0x3) == 0) return -1;

    CAN_FIFO_Mailbox_TypeDef *mb = &CAN1->RX[0];

    // Parse RIR: extract ID, IDE, RTR.
    uint32_t rir = mb->RIR;
    if (rir & (1 << 2)) {
        // Extended ID
        frame->ide = 1;
        frame->id = (rir >> 3) & 0x1FFFFFFF;
    } else {
        // Standard ID
        frame->ide = 0;
        frame->id = (rir >> 21) & 0x7FF;
    }
    frame->rtr = (rir & (1 << 1)) ? 1 : 0;

    // DLC from RDTR bits [3:0].
    frame->dlc = mb->RDTR & 0xF;

    // Unpack data bytes from RDLR and RDHR.
    uint32_t low  = mb->RDLR;
    uint32_t high = mb->RDHR;
    frame->data[0] = (low  >> 0)  & 0xFF;
    frame->data[1] = (low  >> 8)  & 0xFF;
    frame->data[2] = (low  >> 16) & 0xFF;
    frame->data[3] = (low  >> 24) & 0xFF;
    frame->data[4] = (high >> 0)  & 0xFF;
    frame->data[5] = (high >> 8)  & 0xFF;
    frame->data[6] = (high >> 16) & 0xFF;
    frame->data[7] = (high >> 24) & 0xFF;

    // Release the FIFO slot by setting RFOM0 (bit 5 of RF0R).
    // This advances the FIFO read pointer to the next message.
    CAN1->RF0R |= (1 << 5);

    return 0;
}