#ifndef IR_REMOTE_H
#define IR_REMOTE_H



typedef enum
{
    IR_CMD_0 = 0xE0,    // (224)
    IR_CMD_1 = 0X50,   //(80)
    IR_CMD_2 = 0XD8,  //(216)
    IR_CMD_3 = 0XF8, //(248)
    IR_CMD_4 = 0X30, //(48)
    IR_CMD_5 = 0xB0, //(176)
    IR_CMD_6 = 0x70, // 112
    IR_CMD_7 = 0x00, // 0
    IR_CMD_8 = 0xF0, // 240
    IR_CMD_9 = 0x98, //152
    IR_CMD_VOL1 = 0xA0, // 160
    IR_CMD_VOL2 = 0X60,  // 96
    IR_CMD_RPT = 0X10,  // 16
    IR_CMD_USD = 0X90, // 144
    IR_CMD_POWER_BUTTON = 0X48, // 72
    IR_CMD_NONE = 0XFF,
}ir_command_e;

void ir_remote_init(void);
ir_command_e ir_remote_get_command(void); // function to get the most recent command from the remote

const char *ir_remote_command_to_string(ir_command_e cmd);



#endif 