#include "io.h"
#include "defines.h"


#define port_offset  (3u)   // value 3'u' can be helped to avoid compiler errors while dealing with some datatype of unsigned int type 
#define port_mask    (0x3u<<port_offset)     // 00011000 bit mask for bit 3 and bit 4
#define pin_mask     (7u)

#define IO_PORT_CNT   (2u)     // total ports 2
#define PIN_CNT_PER_PORT (8u)    // pins per port = 8 pins per port
#define IO_INTERRUPT_PORT_CNT (2u)



static inline uint8_t io_pin_idx(io_e io)    // pin number
{
    return io & pin_mask;        // (enum value & (0x7 or 111)) to determine pin number  . the value can be (0-7)
}

static inline io_pin_bit(io_e io)
{
    return (0x1<< io_pin_idx(io));    // can be anyoneone of teh 8 bits useful for the register configs : 0x00000000 , using left shift op make any one bit 1 from these 8 bits
}

static inline uint8_t io_port(io_e io)  // determine port number
{
    return(io & port_mask)>>port_offset;  // this will will return either 0 or 1 ; 0 means port 1 and 1 means port 2 since my launchpad has 2 ports only
}

typedef enum
{
    IO_PORT1,
    IO_PORT2,
}io_port_e;


  // Using arrays and array indexing to avoid big if or swtich statements ;
/* TI's helper header (msp430.h) provides defines/variables for accessing the
 * registers, and the address of these are resolved during linking. For cleaner
 * code, smaller executable, and to avoid mapping between IO_PORT-enum and these
 * variables using if/switch-statements, store the addresses in arrays and access
 * them through array indexing. */
static volatile uint8_t *const port_dir_regs[IO_PORT_CNT] = {&P1DIR,&P2DIR};   // IO_PORT_CNT  mentioned above using define which is 2u for launchpad
static volatile uint8_t *const port_ren_regs[IO_PORT_CNT] = {&P1REN,&P2REN};
static volatile uint8_t *const port_out_regs[IO_PORT_CNT] = { &P1OUT, &P2OUT };
static volatile uint8_t *const port_in_regs[IO_PORT_CNT] = { &P1IN, &P2IN };
static volatile uint8_t *const port_sel1_regs[IO_PORT_CNT] = { &P1SEL, &P2SEL };
static volatile uint8_t *const port_sel2_regs[IO_PORT_CNT] = {&P1SEL2,&P2SEL2};
static volatile uint8_t *const port_interrupt_flag_regs[IO_INTERRUPT_PORT_CNT] = {&P1IFG,&P2IFG};
static volatile uint8_t *const port_interrupt_enable_regs[IO_INTERRUPT_PORT_CNT] = {&P1IE, &P2IE};
static volatile uint8_t *const port_interrupt_edge_select_regs[IO_INTERRUPT_PORT_CNT] = {&P1IES,&P2IES};        



static const struct io_config io_initial_configs[IO_PORT_CNT * PIN_CNT_PER_PORT] =
{
    [IO_TEST_LED] = {IO_SELECT_GPIO,IO_RESISTOR_DISABLED,IO_DIR_OUTPUT,IO_OUT_LOW},



};



void io_configure(io_e io, const struct io_config *config)   // used for the test function
{
    io_set_select(io, config->select);
    io_set_direction(io, config->dir);
    io_set_out(io, config->out);
    io_set_resistor(io, config->resistor);
}


void io_set_select(io_e io,io_select_e select)
{
    const uint8_t port = io_port(io);   // do this one time to reduce machine code ,clock cycle (since one nested swtich case less for each case )
    const uint8_t pin = io_pin_bit(io);     // will give pin bit value for the register to do & or | operation  

    switch (select)
    {
        case IO_SELECT_GPIO:
        *port_sel1_regs[port] &=~pin;
        *port_sel2_regs[port]&=~pin;
        break;

        case IO_SELECT_ALT1:
        *port_sel1_regs[port] |=pin;
        *port_sel2_regs[port]&=~pin;
        break;

        case IO_SELECT_ALT2:
        *port_sel1_regs[port] &=~pin;
        *port_sel2_regs[port]|=pin;
        break;

        case IO_SELECT_ALT3:
        *port_sel1_regs[port]|=pin;
        *port_sel2_regs[port]|=pin;
        break;

    }


    /* one method i can use is switch statements
    switch (select)
    {
    case : IO_SELECT_GPIO
    switch(io){
    case :0
    P1SEL &=~io_pin_bit(io)   // make that bit 0 in the p1sel register for gpio mode
    P1SEL2 &=~io_pin_bit(io)   //  make that bit 0 in the p1sel2 register  for gpio mode

    case:1  // for port 2 registers
     P2SEL &=~io_pin_bit(io)   // make that bit 0 in the p1sel register for gpio mode
     P2SEL2 &=~io_pin_bit(io)   //  make that bit 0 in the p1sel2 register  for gpio mode
     // so on and so forth makes the code very big and messy */
}

void io_set_direction(io_e io, io_dir_e direction)
{
   const uint8_t port = io_port(io);
   const uint8_t pin = io_pin_bit(io);

   switch (direction)
   {
    case IO_DIR_INPUT:
        *port_dir_regs[port] &=~pin;
     break;
    case IO_DIR_OUTPUT:
         *port_dir_regs[port] |=pin;
    break;
        }
}

void io_set_resistor(io_e io,io_resistor_e resistor)
{
        const uint8_t port = io_port(io);
       const uint8_t pin = io_pin_bit(io);
    switch (resistor) {
    case IO_RESISTOR_DISABLED:
        *port_ren_regs[port] &= ~pin;
        break;
    case IO_RESISTOR_ENABLED:
        *port_ren_regs[port] |= pin;
        break;
    }

}
void io_set_out(io_e io, io_out_e out)
{
     const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (out) {
    case IO_OUT_LOW:
        *port_out_regs[port] &= ~pin;
        break;
    case IO_OUT_HIGH:
        *port_out_regs[port] |= pin;
        break;
    }
}

io_in_e io_get_input(io_e io)   // we will use this function to return whether 0 or 1 ,
{
    return(*port_in_regs[io_port(io)] & io_pin_bit(io)) ? IO_IN_HIGH : IO_IN_LOW;   // IF THE BIT IS SET THEN return IO_IN_HIGH OR return IO_LOW
}

void io_init(void)
{
    uint8_t i;
    for (i = 0; i < (IO_PORT_CNT * PIN_CNT_PER_PORT); i++)
    {
        io_configure(i, &io_initial_configs[i]);
    }
}
