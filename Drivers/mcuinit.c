#include "mcuinit.h"
#include "msp430.h"


static void watchdog_stop(void)
{
    /*watchdog is enabled by defualt and will reset the microcontroller repeatedly  if
          not explicitly stopped*/

       WDTCTL = WDTPW + WDTHOLD;   // watchdog stopped
}



void mcu_init()
{
    watchdog_stop();   // must stop the watchdog before anything else
    io_init();     // initializing all the io pins
}
