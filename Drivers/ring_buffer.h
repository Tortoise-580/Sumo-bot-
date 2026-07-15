#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "stdbool.h"
#include"stdint.h"


// Circular buffer (FIFO) that can store elements of any size.

struct ring_buffer      
{
    uint8_t *buffer;    // pointer to the ring buffer
    uint8_t size;       // size variable : since it is a 8 bit variable so max value == 255
    uint8_t head_index;   // index of the next empty slot 
    uint8_t tail_index; // index of the oldest element waiting to be read if not read 
    bool full; // head == tail when full and empty, use this to distinguish
};

void ring_buffer_put(struct ring_buffer *rb,uint8_t  data);   // used for puttng value inside the ring buffer
uint8_t ring_buffer_get(struct ring_buffer *rb);      // reading value from the ring buffer
uint8_t ring_buffer_peek(const struct ring_buffer *rb);   // only peeking inside the ring buffer but not changing anything so putting const struct ring buffer

bool ring_buffer_empty(struct ring_buffer *rb);  // checking whether the ring buffer is empty or not 
bool ring_buffer_full(struct ring_buffer *rb);   // checking whether the ring buffer is full or not 





#endif // RING_BUFFER_H