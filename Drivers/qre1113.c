#include "qre1113.h"
#include "adc.h"
#include "io.h"


// static bool initialised = false;
void qre1113_init(void)
{
   //ASSERT(!initialised)
   
    adc_init();

    // initialised = true;
}


void qre1113_get_voltages(struct qre1113_voltages *voltages)
{
    adc_channel_values_t values;   // size 8 array of uint16_t values initialised
    adc_get_channel_values(values);  // get those sensor ouput voltage in the values array at specific indexes as implemented in the adc.c file 

    //put all the sensor values obtained in this struct
    voltages->front_right = values[io_to_adc_index(IO_LINE_DETECT_FRONT_RIGHT)];
    voltages->front_left = values[io_to_adc_index(IO_LINE_DETECT_FRONT_LEFT)];
    voltages->back_right = values[io_to_adc_index(IO_LINE_DETECT_BACK_RIGHT)];
    voltages->back_left = values[io_to_adc_index(IO_LINE_DETECT_BACK_LEFT)];
}