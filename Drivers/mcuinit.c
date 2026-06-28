#include "mcuinit.h"
#include "msp430.h"





static inline void init_clocks()
{
    /* There are some variations between individual units, so TI calibrates
     * each unit during manufacturing and stores the calibration value in
     * memory to achieve a similar clock rate between different units. Sanity
     * check that the calibration data has not been erased. */
     
  //  ASSERT(CALBC1_1MHZ != 0xFF && CALBC1_16MHZ != 0xFF);

    /* Configure the internal oscillator (main clock) to run at 16 MHz.
     * This clock is used as a reference to produce a more stable DCO. */
    BCSCTL1 = CALBC1_16MHZ;

    // Sets the clock rate of the digitally controlled oscillator (DCO)
    DCOCTL = CALDCO_16MHZ;

    /* Set DCO as source for
     * MCLK: Master clock drives the CPU and some peripherals
     * SMCLK: Subsystem master clock drives some peripherals */
    // BCSCTL2 default

    // Select the internal Very Low Frequency oscillator (VLO) as ACLK source
    BCSCTL3 = LFXT1S_2;
}


static void watchdog_stop(void)
{  
    /*watchdog is enabled by defualt and will reset the microcontroller repeatedly  if 
          not explicitly stopped*/

       WDTCTL = WDTPW + WDTHOLD;   // watchdog stopped
}



void mcu_init()
{     
    watchdog_stop();   // must stop the watchdog before anything else 

  //  io_init();     // initializing all the io pins   

      init_clocks();    // make the clock to 16 mhz 
   
     // Enables  interrupt globally
    _enable_interrupts();
}