#ifndef DEFINES_H
#define DEFINES_H

#define UNUSED(x)  (void)(x)  // here the x is like the placeholder thing like used in functions , whenever this  macro is called that x will take the value

#define MODULO_2(x)   (x&1)   // to take modulo operator with 2 is expensive so just do "&" operation with 1 gives the same result

#define ABS(x)    ((x) >= 0 ? (x) : -(x))   // taking the absolute value 

#define SUPPRESS_UNUSED  __attribute__((unused))
#define ARRAY_SIZE(array) = (sizeof(array)/sizeof(array[0])) 

#define INTERRUPT_FUNCTION(vector) void  __attribute__((interrupt(vector))) 

// increasing the clock speed 
#define CYCLES_1MHZ (1000000u)
#define CYCLES_16MHZ (16u * CYCLES_1MHZ)
#define CYCLES_PER_MS (CYCLES_16MHZ / 1000u)
#define ms_TO_CYCLES(ms) (CYCLES_PER_MS * ms)
#define BUSY_WAIT_ms(ms) (__delay_cycles(ms_TO_CYCLES(ms)))


#define MCLK    (CYCLES_16MHZ)
#define SMCLK   MCLK




#endif
