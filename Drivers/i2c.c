#include "i2c.h"
#include "io.h"
#include "msp430.h"

#define DEFAULT_SLAVE_ADDRESS  (0X29)  // (its 52h in the datasheet but this is 29h is just 7 bit address)
#define COUNT (UINT16_MAX)  // given in stdint.h   0xffff // we keep count the max value possible for convenience

static i2c_result_e i2c_wait_and_check_start_condition(void)
{
    uint16_t count = COUNT;
    while((UCB0CTL1 & UCTXSTT) && count--){}  // the while loop will wait for "COUNT" iterations after start condition was requested ,so UCTXSTT is automatically cleard after start condition is generated 

    if(count==0) // if timeout happened then count ==0 ; hence return this 
    {
        return I2C_RESULT_ERROR_TIMEOUT;
    }
    return (UCB0STAT & UCNACKIFG) ? I2C_RESULT_ERROR_START : I2C_RESULT_OK;  // anyways a NACK or ACK is not expected during START condition  , so if you reach these return statement it will always return I2C_RESULT_OK  
      
}

static i2c_result_e i2c_wait_tx_byte(void)
{
     uint16_t count = COUNT;
    while(!(IFG2 & UCB0TXIFG) && count-- ){}  // check the TXIFG flag , when TXBUF empty TXIFG =1; until then TXIFG = 0

    if(count == 0)
    {
        return I2C_RESULT_ERROR_TIMEOUT;  // timeout has happened and TXIFG was never set 
    }

    return (UCB0STAT & UCNACKIFG) ? I2C_RESULT_ERROR_TX : I2C_RESULT_OK;  // check for  NACK Bit
}
static i2c_result_e i2c_send_address(uint8_t *addr, uint8_t addr_size)
{
    // CONFIGURE as sender/Transmitter
    UCB0CTL1 |= UCTR;

    // Send start conditon (and slave address)  // in msp430 with start condition the addr is also sent
    UCB0CTL1 |= UCTXSTT;   // generate start condition 

    //Note: when the master is Transmitter :  we must write to TXBUF before waiting  for UCTXSTT(like we do in stm32 to wait for start condition to be succesful )
    UCB0TXBUF = addr[0]; // write to TXBUF as msp demands it before checking for start condition 
    i2c_result_e result = i2c_wait_and_check_start_condition();  //  start condition has already been executed (line 25) and recieve the enum value (whether it was succesful or not);

    if(result){  // if any error with the start_conditoin  then return that enum error value
        return result;
    }

    // Now that start conditon has been checked , now check whether the TXBUF is empty or not( we had written addr[0] to it(line 28) before checking start condition)
     result =  i2c_wait_tx_byte();

    if(result){  // if any error with the first tx byte transmission
        return result;  // return this error message
    }

    /* Now i have transmitted 1 byte , so for multi byte/ MULTI BYTE ADDRESS transmission using for loop :-> Although vl530lx does not need this since it has 1 byte register addresses only */

    uint8_t i;
    for(i=1;i<addr_size;i++)  // i  = 1 since since 1 byte has been already been transmitted
    {
        UCB0TXBUF = addr[i];  // transmit the next byte
        result = i2c_wait_tx_byte();
        if(result){  // if any error then return that error enum value in result variable
            return result;
        }
    }
     
    return result;  // if no error then (enum value  = 0;) return I2C_RESULT_OK ;

}

static i2c_result_e i2c_start_tx_transfer(uint8_t *addr, uint8_t addr_size)
{
    return i2c_send_address(addr, addr_size);
}


static i2c_result_e  i2c_stop_transfer(void)
{
    uint16_t count = COUNT;  // count for timeout
    UCB0CTL1 |= UCTXSTP;   // generate stop conditon 

    while((UCB0CTL1 & UCTXSTP) && count--){}  // wait till stop condition is succesfully generated , if generated then UCTXSTP will be cleared automatically
    if(count == 0){
        return I2C_RESULT_ERROR_TIMEOUT;  // if timeout happened return this 
    }

    return (UCB0STAT & UCNACKIFG) ? I2C_RESULT_ERROR_STOP : I2C_RESULT_OK;  // although no need of ack/nack after start/stop conditon  
}

i2c_result_e i2c_write(const uint8_t *addr, uint8_t addr_size, const uint8_t *data, uint8_t data_size)
{
    i2c_result_e result = i2c_start_tx_transfer(addr, addr_size);  // this function will take the addr to get the transmission started

    if(result){  // I2C_RESULT_OK = enum value =  0(means everything okay);  so if everythin is okay then skip this "if block"
    return result;  // if some error occured then return that error enum  value stored in result variable
    }

    /* Now that i have reached here  it means  SLAVE address , memory address has been sent ,and result =I2C_RESULT_OK, now time to send data that we want to send*/

    uint8_t i;  // assuming no of data bytes to be transmitted  is <=255 else take uint16_t\

    // Send from Most to least significant byte (because the range sensor expects like this )
    for(i=0; i<data_size; i++)  // now sending the data to the slave
    {   
        UCB0TXBUF = data[i];
        result = i2c_wait_tx_byte();  
        if(result){  // if any error then return result;
            return result;
        }

    }
        result = i2c_stop_transfer();  // after writing initiate the stop condition 
        return result;
}












static i2c_start_rx_transfer(uint8_t *addr, uint8_t addr_size)
{
    i2c_result_e result = i2c_send_address(addr, addr_size);  // send the slave/memory address to start the rx transmission
    if(result){   // if any error return that error
        return result;
    }

    // Now configure as reciever 
    UCB0CTL1 &=~UCTR;

    UCB0CTL1 |= UCTXSTT;   // generate start condition 

    result = i2c_wait_and_check_start_condition();  // wait for start conditon to be generated succesfully
    if(result){ // if any error with start conditon return that error
        return result;
    }   
    return result;
}

static i2c_wait_rx_byte(void)
{
    uint16_t count = COUNT;
    while(!(IFG2 & UCB0RXIFG) && count--){}   // wait for the rx flag to be set 
    if(count==0){
        return I2C_RESULT_ERROR_TIMEOUT;
    }
    return (UCB0STAT & UCNACKIFG) ? I2C_RESULT_ERROR_RX : I2C_RESULT_OK;
}

i2c_result_e i2c_read(const uint8_t *addr, uint8_t addr_size, uint8_t *data, uint8_t data_size)
{
    i2c_result_e  result = i2c_start_rx_transfer(addr, addr_size);  // start the transmission of slave / memory address 
    if(result){ // if any error then return that error in result
        return result;
    }

    //Read bytes from most to least significat Byte
    uint8_t i;

    for(i= data_size-1; i >= 1; i--)  // for going from the most to least significant byte except the last byte  , because a stop conditon is required before i recieve the last byte
    {
        result = i2c_wait_rx_byte(); // wait for the rx byte to come in the rxbuf
        if(result){ // if any error then return that error
            return result;
        }

        data[i] = UCB0RXBUF;  // place the 
    }

    result = i2c_stop_transfer();  // stop condition request before the last byte as given in the datasheet. this also sends the signal to i2c peripheral to send nack after the last byte , and this ensures stop condition is done . that is why by the time i am waiting for the last byte RXIFG is already set and stop conditon has already been succesfully generated
    if(result){// any error with  stop conditon , return that error
        return result;
    }

    result = i2c_wait_rx_byte(); // Wait for the last byte(although most likely it has already recieved in the RXBUF (since the stop condition has been generated succesfully))
    if(result){ //  if any error then return that error
        return result;
    }
    data[0] =  UCB0RXBUF;  // recieve the last byte succesfully

// if till here everything okay , then return I2C_RESULT_OK
    return I2C_RESULT_OK;

}

/* Wrapper functions for I2C READ AND WRITE for  convenience */

i2c_result_e i2c_read_addr8_data8(uint8_t addr, uint8_t *data)
{
   return i2c_read(&addr,1,data,1);  //address size = 1 byte , and recieve data also 1 byte
}

i2c_result_e i2c_read_addr8_data16(uint8_t addr, uint16_t *data)
{
    return i2c_read(&addr,1,(uint8_t*)data,2);  // recieve 2 bytes of data
}

i2c_result_e i2c_read_addr8_data32(uint8_t addr, uint32_t *data)
{
    return i2c_read(&addr,1,(uint8_t*)data,4);  // recieve 4 bytes of data
}

i2c_result_e i2c_write_addr8_data8(uint8_t addr, uint8_t data)
{
    return i2c_write(&addr,1,&data,1);
}




void i2c_set_slave_address(uint8_t saddr)
{
    UCB0I2CSA = saddr;  // put slave address on this register;
}


static void io_init(void)
{
    static const struct io_config i2c_config ={
    .select = IO_SELECT_ALT3,
    .resistor = IO_RESISTOR_DISABLED,
    .dir = IO_DIR_OUTPUT,
    .out = IO_OUT_LOW
};
    io_configure(IO_16,&i2c_config);   // P1.6 as I2C MODE which is SCLK
    io_configure(IO_17,&i2c_config);   // P1.7  as I2C MODE whic is SDA 
}

// static bool initialised = false;
void i2c_init(void)
{
    //ASSERT(!initialised)

    io_init();  // P1.6 AND P1.7 configuration

    UCB0CTL1 |= UCSWRST;  // RESET PERIPHERAL is needed before configuring 

    // Single master, synchronous mode(since we are using clock), I2C mode
      UCB0CTL0 = UCMST + UCSYNC + UCMODE_3;
    
     

      UCB0CTL1 |= UCSSEL_2;  // Clock selection as SMCLK for i2c peripheral 

    // SMCLK /160 -> clock speed ~= 100khz
      UCB0BR0 = 160;   // clock divider 
      UCB0BR1 = 0;

     UCB0CTL1 &=~UCSWRST;  // Clear Reset

    // Set slave address
        i2c_set_slave_address(DEFAULT_SLAVE_ADDRESS);   // slave address has been written to this register 
   // initialised = true;
}
