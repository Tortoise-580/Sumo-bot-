#include "adc.h"
#include "io.h"
#include "msp430.h"
#include "defines.h"


static const io_e *adc_pins;    // this variable will store the address of the adc_pin_array[];
static uint8_t adc_pin_count;  // pin count which is 4 in this case 

static volatile adc_channel_values_t adc_dtc_block;  // an array of size 8 holding uint16_t values where the dtc will store the channel values ; -> since the hardware is touching this , i want to keep this volatile to avoid compiler optimization   
static volatile adc_channel_values_t adc_dtc_block_temperory;   
static uint8_t dtc_channel_count;

static inline void adc_enable_and_start_conversion(void)
{
    return ADC10CTL0 |=ENC |ADC10SC;  // the enable conversion bit(ENC) AND adc start conversion bit (ADC10SC)
}


static struct io_config initial_adc =   // Overriden by ADC, so just default it to floating input here

{
        .select = IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_DISABLED,
        .dir = IO_DIR_INPUT,
        .out = IO_OUT_LOW,
};

static void initial_adc_io_init(void)
{
    io_configure(IO_13,&initial_adc);  // Overriden by ADC, so just default it to floating input here

}


 // static bool initialised = false;
void adc_init(void)
{
 // ASSERT(!initialised)
 
    initial_adc_io_init();  // do the initial initialization which will be overwritten by adc afterwards

    adc_pins = io_adc_pins(&adc_pin_count);  // now adc_pins which is a pointer variable contains the address of io_adc_pin_array[]; and also the adc_pin_count variable has the size of this array now which is 4 in my case

    uint8_t last_index = 0;
    uint8_t adc10ae0 = 0;
    uint8_t i;
            for(i=0;i<adc_pin_count;i++)
            {
                const uint8_t pin_index = io_to_adc_index(adc_pins[i]);   // check each pin of the adc_pins_array
                const uint8_t pin_bit = 1<< pin_index;  // pin_index converted to pin position 
                adc10ae0 |= pin_bit;  // these pins are now configured as analog pins 

                if(pin_index > last_index){
                    last_index = pin_index;  // assign the greatest pin_index value to the last_index variable as required by the inch variable which is 5 in my case since P1.5 is the last ir sensor so "5" is the greatest value and adc will sample from A5 TO A0
                }
             }
    const uint16_t inch = last_index << 12;   // since in ADC10CTL1 the inch bits are bits 15,14,13,12 :-> so configure these bits . for my case the last_index is 5 (0101) so it is 5<<12 or (0101<< 12)
            

    /* inch: Select channels (last channel when CONSEQ_1)  : as mentioned in the datasheet
     * ADC10DIV_7: Clock division (higher means slower)
     * CONSEQ_1: Sequence of channels mode
     * SHS_0: ADC10SC bit starts conversion  // because there is some other work in the isr , so after doing that work start the next iteration of conversion of channels 
     * ADC10SSEL_1: ACLK as clock source (Slow) * :-> lower clock means more sampling time , more conversion time  , fewwer interrupts  and hence lower power and lower cpu involvement*/
    ADC10CTL1 = inch + ADC10DIV_7 + CONSEQ_1 + SHS_0 + ADC10SSEL_1;

    /* ADC10ON: Enable ADC
     * SREF_0: Voltage reference (VCC and VSS)  for the msp430 it is 3.3v which powers msp430 and so it is the vref
     * ADC10SHT_3: 16 * ADC10CLK sample and hold time (better readings?)
     * MSC: Multiple sample conversion   :-> Trigger once and then it wil automatically convert the next channels in the sequence
     * ADC10IE: Enable interrupt */
    ADC10CTL0 = ADC10ON + SREF_0 + ADC10SHT_2 + MSC + ADC10IE;

     // Enable ADC pins   -> this configuration ovrwrites the nomral gpio configuration that we did before 
    ADC10AE0 = adc10ae0;   // each bit represent A0 - A7 in this register whatever bits are selected as analog pin /ADC pin 

        /* Use data transfer controller (DTC) to transfer data DMA-style from
            * the sampled channels and interrupt afterwards. Note, CONSEQ_1 iterates
            * the channels contiguously from last idx to 0. */   // :-> this means that if i have configured A1,A2,A5 as my adc pins , so inch value = 5  , then the adc will start from a5 ,a4,a3,a2,a1,a0 . and sample all these pins , althought the ADC10AE0 bit value for PIN 0,3,4 is 0 THE ADC will still go andn sample (it will close its switch for the capacitor to store the voltage input value ) for say 16 clock cycles and then store that value even though those values are of no use to us . so to counter this we should configure our pin in that manner like P1.3,P1.4,P1.5 so that there is no gap in between 

         dtc_channel_count = last_index +1;   // total number of channels values that the dtc will have to read and transfer to memory (+1  because index 0 is also included)

         ADC10DTC0 |= ADC10CT;  // :-> enable continous conversion , like dtc_channel_count is 4 , if don't enable continous mode then , when the dtc starts and does the transfer of 1st sequence that is 4 channels , then according to this state machine it is done ( it has internal counter which will store 4) , and then it will stop , and i would again have to store dtc_count in the ADC10DTC1 register , in continous mode i don;t need that , it will again and again transfer the entire sequence to ram 

         ADC10DTC1 = dtc_channel_count;   // number of transfers we want the dtc to make in every sequence , in my case it is 4 since there are 4 channels/ sensors

         ADC10SA = (uint16_t)adc_dtc_block;  // converting the address value from pointer type to integer type
         
        adc_enable_and_start_conversion();
            // initialised = true;
}

INTERRUPT_FUNCTION(ADC10_VECTOR) isr_adc_10(void)  // a sequence has been converted (the dtc has stored those values to ram also ) and now this interrupt has been fired 
{   P1DIR|=BIT6; // only for debugging , to check whether this isr is running or not , because the pausing the code and resuming the code is making this isr non functional 
    P1OUT^=BIT6;
    uint8_t i;
    for(i=0;i < dtc_channel_count;i++){
    adc_dtc_block_temperory[i] = adc_dtc_block[dtc_channel_count - 1 - i];  // since the dtc has written the channel sample in the opposite order 
    // so adc_dtc_block_temperory[0] = adc_dtc_block[5] and here P1.0 value is present . so now P1.0 value is present at array[0] and not array[5]
    }
    adc_enable_and_start_conversion();  // once the data has been obtained in the temperory array that i have created so start conversion with the next sequence

}

void adc_get_channel_values(adc_channel_values_t values)  // fill the application type arrray -> "values"  with sensor values , here i am using a new variable values to fill , i could have done a return adc_dtc_temperory thing also to avoid a new array , but it is not good practice to expose my storage memory to the application , instaed the application should be more like give me a contained to this api and i will fill it whatever you need . Moreover in this function i am only filling the values of those pins which are used as adc pins , so like only 4 pins and rest ignored (so a more optimized functionality)
{ 
    // _disable_interrupts();
    ADC10CTL0 &=~ADC10IE;  // disable adc interrupt  because we are dealing with adc_dtc_cache array which will change if interrupt is fired , as it is involved in the isr
    uint8_t i;

    for(i=0;i<adc_pin_count;i++)    // pin count is 4               
    {
        const uint8_t channel_index = io_to_adc_index(adc_pins[i]);  // take the channel index from the adc_pins_array so get the P1.0,P1.3,P1.4,P1.5 (0,3,4,5) index values 
        values[channel_index] = adc_dtc_block_temperory[channel_index];   // now the array "values" is filled with the sequence values , like only index 0,3,4,5 has been filled rest we did not even touch
    }


 ADC10CTL0 |=ADC10IE;  // disable adc interrupt  because we are dealing with adc_dtc_cache array which will change if interrupt is fired , as it is involved in the isr

// _enable_interrupts();

}