#include "ring_buffer.h"





void ring_buffer_put(struct ring_buffer *rb,uint8_t data)   // used for puttng value inside the ring buffer
{
     rb->buffer[rb->head_index] = data;
     rb->head_index++;   // after putting value to the ring buffer increment to the next index 

    // rb->head_index = rb->head_index % rb->size;   : Can do this also but MODULO operation is very expensive 
     if(rb->head_index == rb->size)  // if head index reached the size value which means wrap around to index 0;
        rb->head_index =0;
}


uint8_t ring_buffer_get(struct ring_buffer *rb)      // reading value from the ring buffer
{
    const uint8_t data = rb->buffer[rb->tail_index];
    rb->tail_index++;  // after extraciting value increment the tail index

     // rb->head_index = rb->head_index % rb->size;   : Can do this also but MODULO operation is very expensive 
     if(rb->tail_index == rb->size)  // if tail index reached the size value which means wrap around to index 0;
        rb->tail_index =0;

        return data;
}

uint8_t ring_buffer_peek(const struct ring_buffer *rb)   // only peeking inside the ring buffer but not changing anything so putting const struct ring buffer
{
    return rb->buffer[rb->tail_index]; // this will only see the value but no change to the tail index value unlike the ring_buffer get funcion 
}

bool ring_buffer_empty(struct ring_buffer *rb)
{
    return rb->head_index == rb->tail_index;    // while extracting data , if the tail index has caught head index that means the buffer is empty i.e there is no new element to read 
}

bool ring_buffer_full(struct ring_buffer *rb)
{
    uint8_t index_after_head = rb->head_index +1;

    if(index_after_head == rb->size)
        index_after_head = 0;

    return index_after_head == rb->tail_index;

}
