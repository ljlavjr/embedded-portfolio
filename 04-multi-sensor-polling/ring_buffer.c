#include "ring_buffer.h"

void ring_buffer_init(volatile RingBuffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
}

bool ring_buffer_put(volatile RingBuffer_t *rb, char c) {
    uint8_t next = (rb->head + 1) % RING_BUFFER_SIZE;
    if (next == rb->tail) return false;
    rb->buffer[rb->head] = c;
    rb->head = next;
    return true;
}

bool ring_buffer_get(volatile RingBuffer_t *rb, char *c) {
    if (rb->head == rb->tail) return false;
    *c = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    return true;
}

bool ring_buffer_is_empty(volatile RingBuffer_t *rb) {
    return (rb->head == rb->tail);
}