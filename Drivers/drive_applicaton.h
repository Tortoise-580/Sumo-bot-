#ifndef DRIVE_APPLICATON_H
#define DRIVE_APPLICATON_H

// A robust drive interface for motor control for the application code 

typedef enum
{
    DRIVE_DIRECTION_FORWARD,
    DRIVE_DIRECTION_REVERSE,
    DRIVE_DIRECTION_ROTATE_LEFT,
    DRIVE_DIRECTION_ROTATE_RIGHT,
    DRIVE_DIRECTION_ARCTURN_SHARP_LEFT,   // left motor speed will be very less compared to right motor to get a sharp left arcturn 
    DRIVE_DIRECTION_ARCTURN_SHARP_RIGHT,  // right motor speed will be very less comapred to left otor to get a shart right arcturn 
    DRIVE_DIRECTION_ARCTURN_MID_LEFT,      // difference of speed between left and right will be less than that of sharp arc turn 
    DRIVE_DIRECTION_ARCTURN_MID_RIGHT,
    DRIVE_DIRECTION_ARCTURN_WIDE_LEFT,
    DRIVE_DIRECTION_ARCTURN_WIDE_RIGHT,   // for a wide turn this differenc of speed between right and left will more less to get a wide turn 

}drive_direction_e;

typedef enum 
{
    DRIVE_SPEED_SLOW,   // less duty cycle for less speed
    DRIVE_SPEED_MEDIUM,
    DRIVE_SPEED_FAST,
    DRIVE_SPEED_MAX,     // the highest duty cycle for max speed 
}drive_speed_e;

void drive_init(void);
void drive_stop(void);   // a functio to stop everything

void drive_set_directon_and_speed(drive_direction_e dir,drive_speed_e speed);  // function to set the drive speed and drive direction 




#endif