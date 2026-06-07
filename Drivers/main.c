#include "io.h"
#include "mcuinit.h"
#include<msp430.h>

static void test_setup(void)
{
    mcu_init();
}


/* static void test_launchpad_io_pins_output(void)   // second test function to test blinking of all the test fu
nction
{   uint8_t i;
    test_setup();
    const struct io_config output_config =
    {
         .dir = IO_DIR_OUTPUT,
        .select = IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_DISABLED,
        .out = IO_OUT_LOW
    };

    for (i=0;i<=15;i++)
    {
        io_configure(i,&output_config);  // all the pin configured as gpio,output mode with pin value = low as o
f now
    }


    while(1)
    {
        for(i=0; i<=15;i++){
            io_set_out(i,IO_OUT_HIGH);
            __delay_cycles(10000);
          io_set_out(i,IO_OUT_LOW);
        }

    }
}*/
 /* static void test_blink_led(void)   // first test function to test a led blink
{
    const struct io_config led_config =
    {
        .dir = IO_DIR_OUTPUT,
        .select = IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_DISABLED,
        .out = IO_OUT_LOW
    };

    io_configure(IO_TEST_LED,&led_config);
    io_out_e out = IO_OUT_LOW;  // initialization of this variable as low initially

       while(1) //    instead of using exor operator we are doing this if else ternary shit
       {
        //  P1OUT ^= (1U<<0);  // COULD HAVE done just this instaed of the below 2 lines

        out = (out==IO_OUT_LOW)? IO_OUT_HIGH : IO_OUT_LOW;   // initially the value is low as done in the initia
lization  , so this tenary operator toggles
         io_set_out(IO_TEST_LED,out);

        __delay_cycles(250000);// 250 ms delay   // if the clock frequ is 1mhz otherwise the delay value might c
hange
      }

}*/

/* Configure all pins except one (pin 1.0) as input with internal pull-up resistors. Configure
 * the exception (pin 1.0) as output to control an LED. Verify by pulling each pin down in
 * increasing order with an external pull-down resistor. LED state changes when the right pin is
 * pulled down. Once all pins have been verified OK, the LED blinks repeatedly.
 *
 * Note, the pins are configured with internal pull-up resistors (instead of pull-down) because
 * some pins on the LAUNCHPAD are already pulled up by external circuitry */


 /*static void test_launchpad_io_pins_input(void)
   { uint8_t i;
    test_setup();
    const struct io_config input_config = {
        .select =IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_ENABLED,
        .dir = IO_DIR_INPUT,
        .out = IO_OUT_HIGH // pull up
    };
    const struct io_config led_config = {
        .select =IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_DISABLED,
        .dir = IO_DIR_OUTPUT,
        .out = IO_OUT_LOW};

    //configure all pins as input
    for(i=0;i<=15;i++)
    {
        io_configure(i,&input_config);
    }
    io_configure(IO_TEST_LED,&led_config);

    for(i=0;i<=15;i++){
        if(i==IO_TEST_LED){
            continue;
        }

        io_set_out(IO_TEST_LED,IO_OUT_HIGH);
        // Wait for user to pull pin low
        while(io_get_input(i)==IO_IN_HIGH){
            __delay_cycles(10000);// 100ms
        }

        io_set_out(IO_TEST_LED,IO_OUT_LOW);
        // wait for user to disconnect
        while(io_get_input(i)==IO_IN_LOW){
            __delay_cycles(10000);// 100ms
        }
    }

    //BLINK LED WHEN THE TEST IS DONE
    while(1){
        io_set_out(IO_TEST_LED,IO_OUT_HIGH);
        __delay_cycles(500000); // 500 ms
        io_set_out(IO_TEST_LED, IO_OUT_LOW);
        __delay_cycles(2000000); // 2000 ms


   }
   }*/


int main()
{
   //WDTCTL = WDTPW + WDTHOLD;    // stop watch dog timer

     // test_blink_led();
  //  test_launchpad_io_pins_output();   // stopping watchdog timer is present inside test_setup which is presen
t inside "test_launchpad_io_pins_output()""

    return 0;
}
