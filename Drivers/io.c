#include "io.h"
#include "defines.h"


// #include"assert.h"

// static_assert(sizeof(io_generic_e) == 1, "Unexpected size, -fshort-enums missing?");


#define port_offset  (3u)   // value 3'u' can be helped to avoid compiler errors while dealing with some datatype of unsigned int type 
#define port_mask    (0x3u<<port_offset)     // 00011000 bit mask for bit 3 and bit 4
#define pin_mask     (7u)

#define IO_PORT_CNT   (2u)     // total ports 2 
#define PIN_CNT_PER_PORT (8u)    // pins per port = 8 pins per port
#define IO_INTERRUPT_PORT_CNT (2u)


 
static inline uint8_t io_pin_idx(io_e io)    // pin number
{
    return io & pin_mask;        // (enum value & (0x7 or 111)) to determine pin number  . the recieved value will  be somewhhere between  (0-7)
}

static inline uint8_t io_pin_bit(io_e io)     
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


static isr_function_pointer isr_functions[IO_PORT_CNT][PIN_CNT_PER_PORT] =   // 2D array to store function pointer values . 16 pins = 2*8 isr addresses 
{
    [IO_PORT1] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},   // initially null addresses 
    [IO_PORT2] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};



/*static const struct io_config io_initial_configs[IO_PORT_CNT * PIN_CNT_PER_PORT] =
{
    [IO_TEST_LED] = {IO_SELECT_GPIO,IO_RESISTOR_DISABLED,IO_DIR_OUTPUT,IO_OUT_LOW},
     
     
    
};*/



void io_configure(io_e io, const struct io_config *config)   // used for the test function 
{
    io_set_select(io, config->select);    // since config is a pointer to a struct  so use -> to dereference it and access the select member  
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

static void io_clear_interrupt(io_e io)             // 3.
{
    *port_interrupt_flag_regs[io_port(io)] &=~io_pin_bit(io);
}


void io_configure_interrupt(io_e io,io_trigger_e trigger,isr_function_pointer isr)                 // 1.
{
    io_set_interrupt_trigger(io,trigger);  // will take io and trigger values /type
    io_register_isr(io,isr);  // if inerrupt has occured for this io pin  register the function address for that pin 

}


/* This function also disables the interrupt because selecting the edge
 * might trigger one according to the datasheet */

static void io_set_interrupt_trigger(io_e io,io_trigger_e trigger)           // 2.
{
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);

    io_disable_interrupt(io);   // disabling interrupt because of datasheet

    switch(trigger) {

        case IO_TRIGGER_RISING: 
        *port_interrupt_edge_select_regs[port]&=~io_pin_bit(io);
        break;

        case IO_TRIGGER_FALLING:
         *port_interrupt_edge_select_regs[port]|=io_pin_bit(io);
         break;

    }
    /* Also clear the interrupt here, because even if interrupt is disabled,
     * the flag is still set */
         io_clear_interrupt(io);   /// this is done because if interrupt is triggered it won't report to cpu because it is disabled but will set the flag in the IFG register . so to start things clean 
}

static void io_register_isr(io_e io,isr_function_pointer isr)           // 4.   REGISETER function address value to the 2d array                 
{
    const uint8_t port = io_port(io);
    const uint8_t pin_idx = io_pin_idx(io);      // will give value between 0-7 just the value in binary ( the shift operator is not applied here )

   //  ASSERT(isr_functions[port][pin_idx] == NULL);   // check if only 1 isr function is assigned per pin 
    isr_functions[port][pin_idx] = isr;  // if asssert clear then assign the function address to this pin


}

void io_deconfigure_interrupt(io_e io,isr_function_pointer isr)    // will not require anything , but just for the sake of completion 
{
    io_unregister_isr(io);
    io_disable_interrupt(io);
}

static void io_unregister_isr(io_e io,isr_function_pointer isr)     // 5.    this function won't be used , just for completion sake 
{
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_idx(io);  

    isr_functions[port][pin] = NULL;    // for unregistering , assign NULL to that pin isr 

    
}



 void io_enable_interrupt(io_e io)       //6.
{
    *port_interrupt_enable_regs[io_port(io)] |=io_pin_bit(io);
}


 void io_disable_interrupt(io_e io)      //7.
{
    *port_interrupt_enable_regs[io_port(io)] &=~io_pin_bit(io);
}

static io_isr(io_e io)     //10
{
    const uint8_t port = io_port(io);      
    const uint8_t pin = io_pin_bit(io);
    const uint8_t pin_idx = io_pin_idx(io);    // 0-7 value range 

    if(*port_interrupt_flag_regs[port] & pin)  // if flag is set then that pin was responsible for the interrupt 
    {
        if(isr_functions[port][pin_idx]!= NULL){
          isr_functions[port][pin_idx]();           // the (); acts like a execution trigger . the cpu pushes the current pc(to return later ) to stack and acts on this new retrieved address 
        }


    io_clear_interrupt(io);  // explicitly clearing interrupt flag  in the software is necessary 

    }


}
INTERRUPT_FUNCTION(PORT1_VECTOR) isr_port_1(void)     // 8.   THE PORT1 ISR , all port 1 isr will be services here .
{    uint8_t io;
    for (io = IO_10; io <= IO_17; io++) {   // using for loop insted of big if else . to check which pin was responsible for the interrupt , the cpu does not know so we are checking each pin using for loop 
        io_isr(io);
    }
}

INTERRUPT_FUNCTION(PORT2_VECTOR) isr_port_2(void)    //9.   // check the 
{       uint8_t io;
    for (io = IO_20; io <= IO_27; io++) {
        io_isr(io);
    }
}
