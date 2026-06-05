#include "io.h"
#include "defines.h"


#define port_offset  (3u)   // value 3'u' can be helped to avoid compiler errors while dealing with some datatype of unsigned int type 
#define port_mask    (0x3u<<port_offset)     // 00011000 bit mask for bit 3 and bit 4
#define pin_mask     (7u)

#define PORT_CNT   (2u)
#define PIN_CNT_PER_PORT (8u)


 
static inline uint8_t io_pin_idx(io_e io)
{
    return io & pin_mask;        // enum value & (0x7 or 111) to determine pin number  . the value can be (0-7)
}

static inline io_pin_bit(io_e io)
{
    return (0x1<< io_pin_idx(io));    // can be anyoneone of teh 8 bits useful for the register configs : 0x00000000 , using left shift op make any one bit 1 from these 8 bits 
}

static inline uint8_t io_port(io_e io)  // determine port number 
{
    return(io & port_mask)<<port_offset;  // this will will return either 0 or 1 ; 0 means port 1 and 1 means port 2 since my launchpad has 2 ports only 
}

typedef enum 
{
    IO_PORT1,
    IO_PORT2,
}io_port_e;

static volatile uint8_t *const port_dir_regs[PORT_CNT] = {&P1DIR,&P2DIR};
static volatile uint8_t *const 
static volatile uint8_t *const
static volatile uint8_t *const
static volatile uint8_t *const

void io_set_select(io_e io,io_select_e select)
{
    UNUSED(io);
    UNUSED(select);
}

void io_set_direction(io_e io, io_dir_e direction)
{
     UNUSED(io);
    UNUSED(direction);
}

void io_set_resistor(io_e io,io_resistor_e resistor)
{
    UNUSED(io);
    UNUSED(resistor);
}
void io_set_out(io_e io, io_out_e out)
{
    UNUSED(io);
    UNUSED(out);
}

io_in_e io_get_input(io_e io)
{
    UNUSED(io);
}