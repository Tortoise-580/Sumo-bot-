#include "pwm.h"
#include "msp430.h"
#include "defines.h"
#include "stdbool.h"
#include "io.h"




/* MSP430G2553 has no dedicated PWM, so use timer A0 to emulate
 * hardware PWM. Each timer has three capture/compare channels
 * (CC) and first channel must be sacrificed for setting the
 * base period (TA1CCR). The two other channels are used for
 * one PWM output each and duty cycle is set by setting the
 * timer value (TA1CCR1 and TA1CCR2). The CC outputs are muxed
 * to corresponding IO pins (see io.h).
 *
 * Example (one period):
 * -----------------_____________ // CC output
 * <----TA1CCRx---->              // Duty cycle
 * <----------TA1CCR0-----------> // Base period
 *
 * Set the base frequency to 20000 Hz because with SMCLK of 16 MHz it
 * gives a base period of 100 ticks, which means the duty cycle
 * percent corresponds to the TA1CCRx directly without any conversion.
 * 20 kHz also gives stable motor behaviour. */



#define  INPUT_DIVIDER_ID_3     (8u)
#define PWM_TIMER_FREQ_HZ     (SMCLK/INPUT_DIVIDER_ID_3)      // clock = 2MHZ for the timerA1 
#define PWM_PERIOD_FREQ_HZ   (20000)   // 20KHZ is the period frequcny which we are setting 
#define PWM_PERIOD_TICKS    (PWM_TIMER_FREQ_HZ / PWM_PERIOD_FREQ_HZ)    // 2MHZ /20K  = 100 ; this is the value we will be using for the CCR0 register which is 100 ticks in up mode 
// static_assert(PWM_PERIOD_TICKS == 100, "Expect 100 ticks per period");

struct pwm_channel_cfg
{  
      bool enabled;   // keeping track whether channel is enabled or not 
      volatile __uint16_t *const CCTL; // const pointer variable to a CAPTURE COMPARE CONTROL REGISTER (CCTL1) &*CCTL2)
      volatile __uint16_t *const CCR;  // const  pointer variable to CAPTURE COMPARE REGISTER : -> used to set the count value for PWM (basically the duty cycle )
};

static struct pwm_channel_cfg pwm_cfgs[] = {    // ARRAY WHOSE ELEMENTS ARE STRUCT OF TYPE pwm_chanel_cfg 

   [PWM_MOTORDRIVER_LEFT] = {.enabled = false,.CCTL=&TA1CCTL1,.CCR = &TA1CCR1},  // by default setting the enabled variable = false 
   [PWM_MOTORDRIVER_RIGHT] ={.enabled = false,.CCTL=&TA1CCTL2,.CCR = &TA1CCR2},
};

static bool pwm_all_channels_timer_disabled(void)
{
   uint8_t ch=0;
   for(ch=0; ch < 2; ch++){
      if(pwm_cfgs[ch].enabled){  // just check for both pwm , if they are enabled 
            return false;    // if enabled then all_channel_disable is false;
      }
   }
   return true;  // if none of them is enabled then return true : all_channels are disabled 
}

static bool pwm_enabled = false;
static void pwm_timer_enable(bool enable)
{  
   if(pwm_enabled != enable){  // just a check so that we don't write this registers unnecessary 
         TA1CTL &= ~(3u<<4);  // clear bit 4 & 5 of this register which is the mode control bits , and now once cleared do the next step 

         TA1CTL|= (enable ?  MC_1 : MC_0); // MODE CONTROL 1 : up mode , so in this mode the timer counts upto TA1CCR0 VALUE , here the TA1CCR0 is kind  of acting like the ARR register  in stm

         TA1CTL |= TACLR;  // this bit clears the counter value (TAR ) , and starts from 0 again , basically reseting the timer counter
   }
}
static void pwm_channel_enable(pwm_e pwm,bool enable)  // make pwm channel enable 
{
   if(pwm_cfgs[pwm].enabled != enable)
   {
      /* OUTMOD_7: Reset/Set
         * OUTMOD_0: Off */
      *pwm_cfgs[pwm].CCTL = (enable) ? OUTMOD_7 : OUTMOD_0;   // if enable == true , then configure the CCTL register 
      pwm_cfgs[pwm].enabled = enable;

      if(enable){
         pwm_timer_enable(true);
      } else if(pwm_all_channels_timer_disabled()){
         pwm_timer_enable(false);
      }    
   }
}

static inline uint8_t pwm_scale_duty_cycle(uint8_t duty_cycle_percent)
{
     /* Battery is at ~8 V when fully charged and motors are 6 V max,
     * so scale down the duty cycle by 25% to be within specs. This
     * should never return 0. */
   return duty_cycle_percent == 1 ? duty_cycle_percent : (duty_cycle_percent >>2)*3;   // if the duty_cycle percent is 1 then will become 0 because of integer round off , so  if duty cycle ==1 then use 1 only no need of *3/4
}

void pwm_set_duty_cycle(pwm_e pwm, uint8_t duty_cycle_percent)
{
      if(pwm == PWM_MOTORDRIVER_RIGHT)  // since on launchpad i can only connect 1 PWM pin /MOTOR DRIVER and which is for the left side 
            return;


   //    ASSERT(duty_cycle_percent<=100);
      const bool enable = duty_cycle_percent > 0;
      if(enable){
         *pwm_cfgs[pwm].CCR = pwm_scale_duty_cycle(duty_cycle_percent);
      }
      pwm_channel_enable(pwm,enable);
}


static const struct io_config pwm_io_config = {
            .select = IO_SELECT_ALT1,  // TIMER A1.1
            .resistor = IO_RESISTOR_DISABLED,
            .dir = IO_DIR_OUTPUT,
            .out = IO_OUT_LOW,
};
// static bool initialised = false;
void pwm_init(void)
{
   // ASSERT(!initialised);

   io_configure(IO_PWM_MOTORS_LEFT,&pwm_io_config);  // configuraton for pin 2.1 as the pwm pin 


      TA1CTL = MC_0;  // TIMER STOPPED MODE 0  
      TA1CTL |=TASSEL_2 | ID_3;   // ->  SMCLK chosen as clock and input divider ID_3 -> SMCLK /8 

      /*SET PERIOD ON CHANNEL 1 TIMER A1*/
      TA1CCR0 = (PWM_PERIOD_TICKS -1);  // 100-1 value given to ccr0 register for setting base period/ frequency 

      // initialised = true;
      
}

 
