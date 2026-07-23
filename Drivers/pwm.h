#ifndef PWM_H
#define PWM_H

#include"stdint.h"

typedef  enum
{
    PWM_MOTORDRIVER_LEFT,  
    PWM_MOTORDRIVER_RIGHT
}pwm_e;


void pwm_init(void);
void pwm_set_duty_cycle(pwm_e pwm,uint8_t duty_cycle_percent);


#endif