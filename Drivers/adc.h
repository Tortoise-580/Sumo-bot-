#ifndef ADC_H 
#define ADC_H
#include <stdint.h>

#define ADC_CHANNEL_COUNT  (8u)

typedef uint16_t adc_channel_values_t[ADC_CHANNEL_COUNT];

void adc_init(void);
void adc_get_channel_values(adc_channel_values_t values);  // to get the latest sensor values 


#endif  