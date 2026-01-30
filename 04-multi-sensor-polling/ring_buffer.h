#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 64

typedef struct {
    char buffer[RING_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
} RingBuffer_t;

void ring_buffer_init(volatile RingBuffer_t *rb);
bool ring_buffer_put(volatile RingBuffer_t *rb, char c);
bool ring_buffer_get(volatile RingBuffer_t *rb, char *c);
bool ring_buffer_is_empty(volatile RingBuffer_t *rb);

#endif