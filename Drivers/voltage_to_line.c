#include "voltage_to_line.h"
#include "qre1113.h"
#include <stdbool.h>

#define VOLTAGE_THRESHOLD (650u)   // obtained from sensor readings on white surface



//static bool initialised = false;
void line_init(void)
{
    //ASSERT(!initialised)

    qre1113_init();

// initialised = true;
}

line_e line_get(void)
{
  struct qre1113_voltages voltages;
  qre1113_get_voltages(&voltages);  // get the 4 voltages and store it  in the struct 

  const bool front_left = voltages.front_left < VOLTAGE_THRESHOLD;   // check whether the sensor are on white surface or not 
  const bool front_right = voltages.front_right < VOLTAGE_THRESHOLD;
  const bool back_left = voltages.back_left < VOLTAGE_THRESHOLD;
  const bool back_right = voltages.back_right < VOLTAGE_THRESHOLD;


        if(front_left){
            if(front_right){
                return LINE_FRONT;
            } else if (back_left){
                return LINE_LEFT;
            } else if (back_right){
                return LINE_DIAGONAL_LEFT;
            } else {
                return LINE_FRONT_LEFT;
            }
        } else if (front_right){
            if(back_right){
                return LINE_RIGHT;
            } else if(back_left){
                return LINE_DIAGONAL_RIGHT;
            } else {
                return LINE_FRONT_RIGHT;
            }
        } else if (back_left){
            if (back_right){
                return LINE_BACK;
           } else {
            return LINE_BACK_LEFT;
           }
        } else if (back_right) {
            return LINE_BACK_RIGHT;
        }else {
            return LINE_NONE;
        }
            

}
