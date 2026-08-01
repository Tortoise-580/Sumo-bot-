#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

// Driver for the motor driver TB6612FNG

#include"stdint.h"

typedef enum   // select motor driver
{
    MOTOR_DRIVER_LEFT,
    MOTOR_DRIVER_RIGHT,
}motor_driver_e;

typedef  enum   // select mode 
{
    MOTOR_DRIVER_MODE_STOP,
    MOTOR_DRIVER_MODE_FORWARD,  // CLOCKWISE ( CW)  -> as given in the datasheet
    MOTOR_DRIVER_MODE_REVERSE,  // ANTI CLOCKWISE (CCw) -> as given in the datasheet
}motor_driver_mode_e;


void motor_driver_init(void);
//void motor_driver_set_mode(motor_driver_e md,motor_driver_mode_e mdm);
//void motor_driver_set_pwm(motor_driver_e md,uint8_t duty_cycle);


#endif
