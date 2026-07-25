#include "drive_applicaton.h"
#include "motor_driver.h"
#include "defines.h"
#include "stdint.h"

struct drive_speeds  // struct to store left and right motor speeds
{
    int8_t left_speed;    // left motor speed
    int8_t right_speed;  // right motor speed
};

/* Drive directions come in pair (e.g. FORWARD and REVERSE, ROTATE_LEFT and ROTATE_RIGHT).
 * To save flash space and minimize typos, only save the speeds for one direction (primary),
 * and create a macro to get the corresponding primary direction for every direction and
 * inverse the speeds when its not the primary direction. */

#define DRIVE_PRIMARY_DIRECTION(dir) (dir - MODULO_2(dir))   // to get the primary DIRECTION ,just do subtract the direction and its modulo , check out the enum value of these directions you will get the primary directions 

static const struct drive_speeds drive_primary_direction_and_their_speeds[][4] = 
{
    [DRIVE_DIRECTION_FORWARD] = {
        [DRIVE_SPEED_SLOW] ={25,25},
        [DRIVE_SPEED_MEDIUM] = {45,45},
        [DRIVE_SPEED_FAST] = {60,60},
        [DRIVE_SPEED_FAST] = {100,100},
    },
    [DRIVE_DIRECTION_ROTATE_LEFT] = {
        [DRIVE_SPEED_SLOW] ={-25,25},
        [DRIVE_SPEED_MEDIUM] = {-45,45},
        [DRIVE_SPEED_FAST] = {-60,60},
        [DRIVE_SPEED_FAST] = {-100,100},        
    },
    [DRIVE_DIRECTION_ARCTURN_SHARP_LEFT] = {
        [DRIVE_SPEED_SLOW] ={-10,25},
        [DRIVE_SPEED_MEDIUM] = {-10,50},
        [DRIVE_SPEED_FAST] = {-25,75},
        [DRIVE_SPEED_FAST] = {-20,100},        
    },
    [DRIVE_DIRECTION_ARCTURN_MID_LEFT] = {
        [DRIVE_SPEED_SLOW] ={15,30},
        [DRIVE_SPEED_MEDIUM] = {25,50},
        [DRIVE_SPEED_FAST] = {40,70},
        [DRIVE_SPEED_FAST] = {50,100},        
    },
    [DRIVE_DIRECTION_ARCTURN_WIDE_LEFT] = {
        [DRIVE_SPEED_SLOW] ={20,25},
        [DRIVE_SPEED_MEDIUM] = {40,50},
        [DRIVE_SPEED_FAST] = {60,75},
        [DRIVE_SPEED_FAST] = {80,100},        
    },
};

static void drive_inverse_speeds(int8_t *speed_left,int8_t *speed_right)
{
        if(*speed_left == *speed_right)  // this is the DRIVE_DIRECTION_REVERSE CASE for which speed is same just we have to reverse the direction for both the speeds 
        {
            *speed_left = -(*speed_left);  // make 25 = -25 (say for DRIVE_SPEED_SLOW) FOR DRIVE_DIRECTION_REVERSE 
            *speed_right = -(*speed_right);  // make 25 = -25 (say for DRIVE_SPEED_SLOW) FOR DRIVE_DIRECTION_REVERSE 
        }
        else{  // if it is not DRIVE_DIRECTION_REVERSE , then just swap the left and right speed and it will work 

           *speed_left ^= *speed_right;
           *speed_right ^= *speed_left;
           *speed_left ^= *speed_right;      

        }
}

void drive_set_direction_and_speed(drive_direction_e direction,drive_speed_e speed)
{
    drive_direction_e primary_direction = DRIVE_PRIMARY_DIRECTION(direction);   // store the primary direction of the given input 

    int8_t speed_left = drive_primary_direction_and_their_speeds[primary_direction][speed].left_speed;   // extract the left speed value for this primary direction 
    int8_t speed_right = drive_primary_direction_and_their_speeds[primary_direction][speed].right_speed; // extract the right speed value for this primary direction 

    if(direction != primary_direction){
        drive_inverse_speeds(&speed_left,&speed_right);
    }

    motor_driver_e mode_left = speed_left > 0 ? MOTOR_DRIVER_MODE_FORWARD : MOTOR_DRIVER_MODE_REVERSE;
    motor_driver_set_mode(MOTOR_DRIVER_LEFT,mode_left);  // the appropiriate mode has been set either forward or reverse for left motor driver

    motor_driver_e mode_right = speed_right > 0 ? MOTOR_DRIVER_MODE_FORWARD : MOTOR_DRIVER_MODE_REVERSE;
    motor_driver_set_mode(MOTOR_DRIVER_RIGHT,mode_right);  //// the appropiriate mode has been set either forward or reverse for left motor driver

    motor_driver_set_pwm(MOTOR_DRIVER_LEFT,ABS(speed_left));    // now the mode (forward or backwared) has been set pwm for left motor driver
    motor_driver_set_pwm(MOTOR_DRIVER_RIGHT,ABS(speed_right));   // now the mode (forward or backward) has been set pwm for right motor driver 
}


void drive_stop(void)
{
    motor_driver_set_mode(MOTOR_DRIVER_LEFT,MOTOR_DRIVER_MODE_STOP);  // using the motor_driver.c functions only here to stop both the motors
    motor_driver_set_mode(MOTOR_DRIVER_RIGHT,MOTOR_DRIVER_MODE_STOP);

    motor_driver_set_pwm(MOTOR_DRIVER_LEFT,0);  // Also setting the duty cycle to 0 
    motor_driver_set_pwm(MOTOR_DRIVER_RIGHT,0);

}

// static bool initialised = false;
void drive_init(void)
{
   // ASSERT(!initialised)
    motor_driver_init();
}