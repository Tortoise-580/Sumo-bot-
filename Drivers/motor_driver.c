#include "motor_driver.h"   
#include "pwm.h"
#include "io.h"


struct CC_PINS
{
    io_e CC1;
    io_e CC2;
};

static struct CC_PINS motor_driver_cc_pins[] = {

    [MOTOR_DRIVER_LEFT] = {.CC1 = IO_MOTORS_LEFT_CC_1, .CC2 = IO_MOTORS_LEFT_CC_2},
    [MOTOR_DRIVER_RIGHT] = {.CC1 = IO_MOTORS_LEFT_CC_1, .CC2 = IO_MOTORS_LEFT_CC_2},  // TO AVOID COMPILATION ERROR
};


 void motor_driver_set_mode(motor_driver_e md,motor_driver_mode_e mode)
 {  
    
    switch (mode) {
    case MOTOR_DRIVER_MODE_STOP:
        io_set_out(motor_driver_cc_pins[md].CC1, IO_OUT_LOW);  // configure cc1 pin according to mode 
        io_set_out(motor_driver_cc_pins[md].CC2, IO_OUT_LOW);  // configure cc2 pin according to mode 
        break;
    case MOTOR_DRIVER_MODE_FORWARD:
    io_set_out(motor_driver_cc_pins[md].CC1, IO_OUT_HIGH);  // as given in the truth table of the datasheet
    io_set_out(motor_driver_cc_pins[md].CC2, IO_OUT_LOW);
        break;

    case MOTOR_DRIVER_MODE_REVERSE:
    io_set_out(motor_driver_cc_pins[md].CC1, IO_OUT_LOW);   // as given in the truth table of the datasheet
    io_set_out(motor_driver_cc_pins[md].CC2, IO_OUT_HIGH);
        break;

    }
 }
// static_assert(TB6612FNG_LEFT == (int)PWM_TB6612FNG_LEFT, "Enum mismatch");
// static_assert(TB6612FNG_RIGHT == (int)PWM_TB6612FNG_RIGHT, "Enum mismatch");
 void motor_driver_set_pwm(motor_driver_e md,uint8_t duty_cycle)
 {
    pwm_set_duty_cycle((pwm_e)md,duty_cycle);
 }


static const struct io_config cc_io_config = {   // CONFIGURATION FOR THE CC PINS WHICH ARE P2.6 AND P2.7 for the left motor driver
    .select = IO_SELECT_GPIO,
    .resistor = IO_RESISTOR_DISABLED,
    .dir = IO_DIR_OUTPUT,
    .out = IO_OUT_LOW,
};
// static bool initialised = false;
void motor_driver_init()
{
//    ASSERT(!initialised)
      
      /* configure the CC PINS for the left motor driver PIN 2.6 & P2.7*/
        io_configure(IO_MOTORS_LEFT_CC_1,&cc_io_config);  // P 2.6 confugure  CC 1  
        io_configure(IO_MOTORS_LEFT_CC_2,&cc_io_config);  // P2.7 configured as CC 2

        pwm_init(); //  pwm initialised here only inside motor_driver_init()

// initialised = true;
}
