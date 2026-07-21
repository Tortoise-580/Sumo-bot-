#include "ir_remote.h"
#include"msp430.h"
#include "defines.h"
#include"stdint.h"
#include"ring_buffer.h"

#define TIMER_TIMEOUT_MS   (150u)


#define TIMER_DIVIDER_ID_3  (8u)     // ID_3 (INPUT DIVIDER 3)  == /8      -> well this value is 20k ticks per ms for smclk = 16mhz
#define TICKS_PER_ms  (16000000/TIMER_DIVIDER_ID_3/1000U)   // 20k ticks per ms 
#define TIMER_INTERRUPT_ms   (1u)    // trigger interrupt after this many milliseconds
#define TIMER_INTERRUPT_TICKS_PER_INTERRUPT   (TICKS_PER_ms * TIMER_INTERRUPT_ms)    // for timer interrupting every 1 ms : it is 20k ticks per interrupt 
// static_assert(TIMER_INTERRUPT_TICKS <= 0xFFFF, "Ticks too large");  //: static assert to check whether the value is less than  FFFF i.e it should fit insdie the 16 bit value since the timer is only 16 bit wide 

static uint8_t timer_ms =0;  // the count variable initialised to 0  : it keeps the count of number of times the timer interrupt happens 
static uint16_t pulse_count = 0;        // this variables tracks the number of pulses / falling edges on P2.0 

#define IR_CMD_BUFFER_ELEMENT_CNT    (10u)  // this buffer of size 10 (can store 10 elements)
static uint8_t buffer[IR_CMD_BUFFER_ELEMENT_CNT];   // BUFFER initialised to store 10 8 bit values 

static struct ring_buffer ir_cmd_buffer = {.buffer = buffer,.size = sizeof(buffer)};   // this ring buffer initialised 



static union{
        struct{
            uint8_t cmd_inverted; 
            uint8_t cmd;
            uint8_t address_inverted;
            uint8_t address;
            }decoded;
            uint32_t raw;
}ir_message;        // variable of this un named type union




static void timer_init()
{
    /* Configure the timer to trigger interrupt after TIMER_INTERRUPT_TICKS
    * TASSEL 2 : SMCLK
    *ID 3 : input divider 8 */
    TACTL |= TASSEL_2 | ID_3 ;    // SMCLK as clock source and input divider /8

    TACCR0 = TIMER_INTERRUPT_TICKS_PER_INTERRUPT ;   // load 20k into this register , so after counter reaches 20k and when compared interrupt will be triggered
    
    TACCTL0 |= CCIE;   // Capture/compare interrupt enable. This bit enables the interrupt request of the corresponding CCIFG flag(which is bit 0 of this register btw).
    
}

static void timer_start(void)   // MC_1  : count to CCR0
{
    TACTL &= ~(3U<<4);   // clear bit 4 & 5 of this register which is the mode control bits , and now once cleared do the next step 
    TACTL |=MC_1;   // MODE CONTROL 1 : up mode , so in this mode the timer counts upto TACCR0 VALUE , which is 20k as of now . here the TACCR0 is kind  of acting like the ARR register  in stm

    TACTL |= TACLR;  // this bit clears the counter value (TAR ) , and starts from 0 again , basically reseting the timer counter

    timer_ms = 0; // since the timer is restarting to measeure time between next falling  edge , so making ths variable again to 0 ;


}

static void timer_stop(void)  // MC_0 : stop counter
{
        TACTL &= ~(3U<<4);   // clear bit 4 & 5 of this register which is the mode control bits , and now once cleared do the next step 
        TACTL |= MC_0;   // MODE CONTROL 0 : STOP THE TIMER    

}

static void isr_pulse(void)  // whenever the falling edge / pulse happens 
{   
    timer_stop();
    pulse_count++;   // this variables tracks the number of pulses / falling edges happening on P2.0

    if(3<=pulse_count && pulse_count<=34)  // if pulse count value is between 3 & 34(included) then decode the bit 
    {
        ir_message.raw <<=1;   // we are bit by bit constructing this 32 bit message 

        ir_message.raw += (timer_ms >=2 ) ? 1 : 0;   // if timer_ms value is >=2 then return 1 else return 0 . that is 2 timer interrupts happened then the bit is 1 else 0
        // like this we will go tlll pulse 34 which means we have recieved all 32 bits 
     }

     if(pulse_count == 34)
     {
        // ONCE WE reciever the full value after 34 falling edges , so now we extract the cmd value and ADD TO THE RING BUFFER  
            ring_buffer_put(&ir_cmd_buffer,ir_message.decoded.cmd);  // adress of the ring buffer given and value from the union given as well (this is our data)
     }

        timer_start(); // will restart the timer , and also restarts the timer_ms count to 0;
}


INTERRUPT_FUNCTION(PORT2_VECTOR) isr_falling_edge_incoming(void)   // this isr is shared between all the 8 pins of port 2
{  
   if(P2IFG & BIT0)  // check if interrupt was raised by  P2.0 . 
   {
      isr_pulse();    // this function is responsible for decodng of the recieved  signal 

      P2IFG &= ~BIT0;   // Clear the interrupt flag 
   }  
}

INTERRUPT_FUNCTION(TIMER0_A0_VECTOR) isr_timer_a0(void)   // every 1ms timer will trigger this 
{
    if(timer_ms < TIMER_TIMEOUT_MS){   // if no new message is coming for 150 ms then transmisson is over  and stop the timer . this 150 ms is taken ino account , since the worst case if of repated code , so it takes around 108 ms like first data frame = 60-70 ms and then wait for another 40ms if button is still pushed which makes it around 110 ms .
    timer_ms++;      // increments each time this interrupt triggers
    } else {
        timer_stop();
        timer_ms = 0;
        pulse_count = 0;
        ir_message.raw = 0;
    }
}


ir_command_e  ir_remote_get_command(void)
{   
    //DISABLE THE INTERRUPT  // since here i am dealing with the ring buffer and after interrupt also the isr is putting data in the ring buffer so it is advisable to disable the interrupt while we are dealing with the here , 
    P2IE &=~ BIT0;

    ir_command_e cmd = IR_CMD_NONE;
    if(!ring_buffer_empty(&ir_cmd_buffer)){
        cmd = ring_buffer_get(&ir_cmd_buffer);
    }
    // ENABLE THE INTERRUPT 
    P2IE|=BIT0;

    return cmd;

    
}




void ir_remote_init()
{   // GPIO input   (IR reciever is connected to this pin )
     P2SEL &= ~BIT0;
     P2SEL2 &= ~BIT0;   // GPIO MODE FOR P2.0 connected to the IR RECIEVER 

     P2DIR &=~ BIT0;   // DIRECTION INPUT FOR P2.0
     P2REN &= ~ BIT0;  // RESISTOR DISABLED AS IR RECIEVER alreads has its own pull up resistor so no need here  


    P2IES |= BIT0;   // falling edge on P2.0
    P2IE  |= BIT0;   // enable interrupt on P2.0
    
    timer_init();
}


const char* ir_remote_command_to_string(ir_command_e cmd)   // the function name is ir_remote_command_to_string which returns pointer to const character 


{
    switch (cmd) {
    case IR_CMD_0:
        return "0";    // these string literals are stored in flash 
    case IR_CMD_1:
        return "1";
    case IR_CMD_2:
        return "2";
    case IR_CMD_3:
        return "3";
    case IR_CMD_4:
        return "4";
    case IR_CMD_5:
        return "5";
    case IR_CMD_6:
        return "6";
    case IR_CMD_7:
        return "7";
    case IR_CMD_8:
        return "8";
    case IR_CMD_9:
        return "9";
    case IR_CMD_VOL1:
        return "VOL1";
    case IR_CMD_VOL2:
        return "VOL2";
    case IR_CMD_RPT:
        return "RPT";
    case IR_CMD_USD:
        return "USD";
    case IR_CMD_POWER_BUTTON:
        return "POWER_BUTTON";
    case IR_CMD_NONE:
        return "NONE";
    }
    return "";
}  
    

