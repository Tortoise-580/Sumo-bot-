#include "uart.h"
#include "msp430.h"
#include "stdint.h"
#include "defines.h"
#include "ring_buffer.h"


#define UART_BUFFER_SIZE  (16)
static uint8_t buffer[UART_BUFFER_SIZE];  // buffer initialised
static struct ring_buffer tx_buffer = {.buffer =buffer,.size = sizeof(buffer)};  // .head_index and .tail_index initialized to 0 ;

static inline void uart_tx_clear_interrupt(void)
{   
   IFG2 &=~ UCA0TXIFG;
}

static inline void uart_tx_interrupt_enable(void)
{
       // Enable  TX interrupt 
         IE2 |= UCA0TXIE;   // IE2 register and this bit so tx interrupt enable whenever tx buffer is empty 
}

static inline void uart_tx_interrupt_disable(void)
{
      // Disable  TX interrupt 
      IE2 &=~ UCA0TXIE;   // IE2 register and this bit so tx interrupt enable whenever tx buffer is empty 
}


static void uart_tx_start(void)
{   
   if(!ring_buffer_empty(&tx_buffer))   // just againt check if not empty then start , load the character in the txbuf
   {
      UCA0TXBUF = ring_buffer_peek(&tx_buffer);   // using peek function here , The byte must stay in the queue until transmission finishes.
   }


}

__attribute__((interrupt(USCIAB0TX_VECTOR))) void isr_uart_tx()    // UART ISR whenever txbuf is empty CPU  will come here
{
   if(ring_buffer_empty(&tx_buffer)){      // if ring buffer empty then wait in the infinite while loop , althought this never happens generally 
      while(1);
   }

   // Remove the transmitted  data byte from the buffer 
      ring_buffer_get(&tx_buffer);

      uart_tx_clear_interrupt();

      if(!ring_buffer_empty(&tx_buffer))   // if ring_buffer not empty  i.e there are remaining elements in the ring buffer then start a new transmission 
      {
         uart_tx_start();
      }
}





/* Calculate the integer and fractional part of the divisor
 * N = (Clock source / Desired baudrate)
 * for Low-Frequency Baud Rate Mode.
 *
 * These are used to configure the desired baudrate.
 *
 * For information on the corresponding transmission error
 * for common values, please refer to the table provided in
 * the family user guide (SLAU144K). */




#define BRCLK (SMCLK)    // datasheet mentions BRCLK so ...     //4.1
#define UART_BAUD_RATE (115200u)                               //4.2
// static_assert(UART_BAUD_RATE < (BRCLK / 3.0f),  "Baudrate must be smaller than 1/3 of input clock in Low-Frequency Mode");   -> as given in the datasheet although we know Baudrate is smaller than 1/3 of clock freqency but still       //  
// this static assert is done because we have to write this value into 16 bit registers so it should fit otherwise the compiler will truncate it and alltogether a wrong value will be used 
            
#define UART_DIVISOR ((float)BRCLK / UART_BAUD_RATE)      // N VALUE   :  here the integer part and fractional part will be extraced         4.4                                          
// static_assert(UART_DIVISOR < 0xFFFFu, "Sanity check divisor fits in 16-bit");                                                             4.5
#define UART_DIVISOR_INT_16BIT ((uint16_t)UART_DIVISOR)     // the integer value from N extracted                                            4.6
#define UART_DIVISOR_FRACTIONAL (UART_DIVISOR - UART_DIVISOR_INT_16BIT) //   fractional part from N extracted                                4.7
#define UART_UCBRS ((uint8_t)(8 * UART_DIVISOR_FRACTIONAL))  // for the modulator as given in the datsheet -> UCBRSx = round( ( N – INT(N) ) × 8 ) : basically (int)fractional part * 8      4.8
// static_assert(UART_UCBRS < 8, "Sanity check second modulation stage value fits in 3-bit"); // THE MODULATOR IS EXPECTING A 3 BIT VALUE so range 0 -7                                      4.9


#define UART_DIVISOR_INT_LOW_BYTE (UART_DIVISOR_INT_16BIT & 0xFF)  // Divide the INT value into LSB AND MSB : this is the lsb obtained my masking lsb 8 bits         4.10
#define UART_DIVISOR_INT_HIGH_BYTE (UART_DIVISOR_INT_16BIT >> 8)    // Divide the INT value into LSB AND MSB : this is the msb                                       4.11









void uart_init(void)
{
    /* Reset module. It stays in reset until cleared. The module should be in reset
     * condition while configured according to the user guide (SLAU144K). */
        UCA0CTL1 |= UCSWRST;                                                                          //1.

    /* Use default (data word length 8 bits, 1 stop bit, no parity bit)
     * [ Start (1 bit) | Data (8 bits) | Stop (1 bit) ] */
        UCA0CTL0 = 0;    // keeping the default mode only                                          //2. 

     // Set SMCLK as clock source.
        UCA0CTL1 |= UCSSEL_2;        // here bit 6 & 7 are being used : 10b = UCSSEL_2          //3.

   // Set the clock prescaler to the integer part of the N in UCABR0 AND UCABR1 register        //5.
   UCA0BR0 = UART_DIVISOR_INT_LOW_BYTE;  // LSB for the prescaler
   UCA0BR1 = UART_DIVISOR_INT_HIGH_BYTE;  // MSB for the prescaler 

   /* Set modulation to account  for the fractional part  of DIVISOR N                         // 6.
       UCA0MCTL = [UCBRF(4 BITS) |UCBRS(3 BITS) | UCOS16(1 BIT)] */
       UCA0MCTL|= (0u<<4)|UART_UCBRS<<1;        // bit 4,5,6,7 =0 , and bit 1,2,3 = UART_UCBRS value for the modulator 
       UCA0MCTL &=~UCOS16;    // UCOS16 ie, bit0 cleared so bit 4,5,6,7 so need to use and their value is 0 as we know 


      // Clear reset to release the module for operation.
            UCA0CTL1 &= ~UCSWRST;

      // Interrupt triggers when TX buffer is empty , which it is after boot, so clear it here .
          uart_tx_clear_interrupt();

      uart_tx_interrupt_enable();  // enable tx inerrupt 
}


void uart_putchar_polling(char c)
{
     while(!(IFG2 & UCA0TXIFG)){}  // wait till tx buffer is empty  : once txbuffer is empty even if interrupt has not been enabled  this UCA0TXIFG flag  which is an interrupt flag will set 
        UCA0TXBUF = c;       // once empty fill it with the next character
}


void uart_putchar_interrupt(char c)
{
      //Poll if full and not proceed further 
      while(ring_buffer_full(&tx_buffer)){}    // althought the mcu running at 16mhz this won't even happen , for big strings also 
 
 
   uart_tx_interrupt_disable();
   const bool tx_ongoing = !ring_buffer_empty(&tx_buffer);   // if not empty then transmissin is ongoing 
   
   ring_buffer_put(&tx_buffer,c);  // put the character in the ring buffer

   if(!tx_ongoing)    // if not ongoing then start the transmission  
   {
      uart_tx_start();
   }

   uart_tx_interrupt_enable();
   
}

void uart_print_interrupt(const char *string)
{
   uint8_t i=0;
   while(string[i] != '\0')
   {
      uart_putchar_interrupt(string[i]);
      i++;
   }
}