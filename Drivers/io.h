#ifndef IO_H
#define IO_H


#include<stdbool.h>
#include<stdint.h>
 #include <msp430.h>
 #include<stdlib.h>

typedef enum 
{
    IO_10,  // port1.0 
    IO_11,IO_12, IO_13,IO_14,
    IO_15,IO_16,IO_17,IO_20,IO_21,IO_22,IO_23,IO_24,IO_25,IO_26, IO_27,
}io_generic_e;


typedef  enum 
{   
    IO_TEST_LED = IO_10,
    IO_UART_RXD = IO_11,
    IO_UART_TXD = IO_12,
    IO_LINE_DETECT_FRONT_LEFT = IO_13,
    IO_UNUSED_2 = IO_14,
    IO_UNUSED_3 = IO_15,
    IO_I2C_SCL = IO_16,
    IO_I2C_SDA = IO_17,
    IO_IR_REMOTE = IO_20,
    IO_MOTORS_LEFT_CC_1 = IO_21,
    IO_MOTORS_LEFT_CC_2 = IO_22,
    IO_RANGE_SENSOR_FRONT_INT = IO_23,
    IO_XSHUT_FRONT = IO_24,
    IO_UNUSED_11 = IO_25,
    IO_PWM_MOTORS_LEFT = IO_26,
    IO_UNUSED_13 = IO_27,
    
}io_e;

typedef enum 
{
    IO_SELECT_GPIO,    // ENUM VALUE : 00
    IO_SELECT_ALT1,    // ENUM VALUE : 01
    IO_SELECT_ALT2,    // ENUM VALUE : 10
    IO_SELECT_ALT3,    // ENUM VALUE : 11
}io_select_e;

typedef enum
{
    IO_DIR_INPUT,
    IO_DIR_OUTPUT,
}io_dir_e;

typedef enum
{
    IO_RESISTOR_DISABLED,
    IO_RESISTOR_ENABLED,
}io_resistor_e;

typedef enum
{
    IO_OUT_LOW,    // (pull down)
    IO_OUT_HIGH   // (pull up)
}io_out_e;

typedef enum
{
    IO_IN_LOW,       // WHETHER THE INPUT IS LOW OR
    IO_IN_HIGH,       // HIGH 
}io_in_e;

typedef enum
{   
    IO_TRIGGER_RISING,   
    IO_TRIGGER_FALLING,

}io_trigger_e;

struct io_config
{
    io_select_e select;     // after implementing the fcompiler flag the memory taken by this 1 byte
    io_resistor_e resistor;
    io_dir_e dir;
    io_out_e out;  
};

void io_configure(io_e io,const struct io_config *config);   // 'const' struct means the pointer won't be able to change the value of the elements of the struct 
void io_set_select(io_e io,io_select_e select);
void io_set_direction(io_e io,io_dir_e direction);
void io_set_resistor(io_e io,io_resistor_e resistor);
void io_set_out(io_e io, io_out_e out);
io_in_e io_get_input(io_e io);  // funcion of return type (io_in_e) which takes io value 


typedef void(*isr_function_pointer)(void);
void io_configure_interrupt(io_e io,io_trigger_e trigger,isr_function_pointer isr);
void io_deconfigure(io_e io);
void io_enable_interrupt(io_e io);
void io_disable_interrupt(io_e io);

static void io_set_interrupt_trigger(io_e io, io_trigger_e trigger);
static void io_register_isr(io_e io,isr_function_pointer isr);


#endif