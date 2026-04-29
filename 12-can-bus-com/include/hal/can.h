#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include "stm32f407.h"

typedef struct {
    uint32_t id;        // 11 bit standard ID or 29 bit extended
    uint8_t ide;        // 0 = standard 11 bit, 1 = extended 29 bit
    uint8_t rtr;        // 0 = data frame, 1 = remote frame
    uint8_t dlc;        // data length code, 0 to 8 bytes
    uint8_t data[8];    // payload bytes
} can_frame_t;

typedef enum {
    CAN_MODE_NORMAL = 0,
    CAN_MODE_LOOPBACK,
    CAN_MODE_LOOPBACK_SILENT,
    CAN_MODE_SILENT
} can_mode_t;

typedef enum {
    CAN_BITRATE_125K = 0,
    CAN_BITRATE_250K,
    CAN_BITRATE_500K,
    CAN_BITRATE_1M
} can_bitrate_t;

int can_init(can_bitrate_t bitrate, can_mode_t mode);
int can_filter_accept_all(void);
int can_transmit(const can_frame_t *frame);
int can_receive_available(void);
int can_receive(can_frame_t *frame);

#endif