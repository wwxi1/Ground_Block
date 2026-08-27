
#include "Ground_Block.h"

uint8_t solenoid_is_open = 0; // 夹爪张开与关闭状态
uint8_t solenoid_enable = 0; // 夹爪使能状态
static volatile uint8_t gb_cmd = 0;   
static uint8_t gb_data[8];    
      

void Ground_Block_Enable()
{
    solenoid_enable = 1;
    DJmotor[GROUND_BLOCK_dji_num].Begin = 1;
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Disable;

}
void Ground_Block_Disable()
{
    solenoid_enable = 0;        // 关闭电磁阀使能
    DJmotor[GROUND_BLOCK_dji_num].Begin = 0;       // 停止电机运行
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Disable; // 设置电机为禁用模式

}

void Ground_Block_Init()
{
    Ground_Block_Disable();
}


/*
void solenoid_reset()
{
    uint8_t data1 = 0x04; //  先收夹爪47
    uint8_t data0 = 0x00; //  再收主杆

    register_updata(&solenoid_Channel1, &data1);
    register_updata(&solenoid_Channel2, &data1);
    register_updata(&solenoid_Channel3, &data1);
    osDelay(2000);
    register_updata(&solenoid_Channel1, &data2);
    register_updata(&solenoid_Channel2, &data2);
    register_updata(&solenoid_Channel3, &data2);
    osDelay(1000);
}
*/

void Ground_Block_reset()
{
   //下降高度，打开夹爪
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set =  DJ_Position;
    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = 0.0f;
    solenoid_on(3, 0x03);
    solenoid_is_open = 1;
    osDelay(1000);
}

void Ground_Block_close()
{
   //先下降高度，再收夹爪,再收主杆
   
    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = 0.0f;
    osDelay(4000);
    solenoid_on(3, 0x4);
    solenoid_is_open = 0;
    solenoid_on(3, 0x00);
    osDelay(1000);
}

void Ground_Block_GetReady() // 预取大地块
{
    // 杆推出，夹爪张开
    solenoid_on(3, 0x7);
    solenoid_is_open = 1;
    osDelay(1000);
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Position;

}

// 接收数据 并执行对应动作，此处osdelay需更改，目前测试用
void Ground_Block_Fetch(uint8_t *Rxdata)
{
     if (solenoid_is_open == 1)
        {
            solenoid_on(3, 0x04); // 夹爪闭合
            osDelay(1000);
        }

    switch (Rxdata[0])
    {
    case 1:

         DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -100.0f;

        break;
    case 2:
        
         DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -250.0f;

        break;
    default:
        break;
    }
}

void Ground_Block_Lay(uint8_t *Rxdata)
{   
    switch (Rxdata[0])
    {
    case 1:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = 0.0f;
        osDelay(2000);
        solenoid_on(3, 0x03); // 夹爪张开
        break;
    case 2:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = 0.0f;
        osDelay(2000);
        solenoid_on(3, 0x03); // 夹爪张开
        osDelay(1000);
        break;
    default:
        break;
    }
}



void Ground_Block_Func(CAN_RxHeaderTypeDef RxHeader, uint8_t *Rxdata)
{
    if (RxHeader.ExtId == 0x01010301)   
    {
        Ground_Block_Enable();          
        return;
    }
    else if (solenoid_enable == 1 && DJmotor[GROUND_BLOCK_dji_num].Begin == 1)
    {
        memcpy(gb_data, Rxdata, 8);
        if (RxHeader.ExtId == 0x01010302) gb_cmd = 2;  
        else if (RxHeader.ExtId == 0x01010303) gb_cmd = 3;  
        else if (RxHeader.ExtId == 0x01010304) gb_cmd = 4;  
        else if (RxHeader.ExtId == 0x010103FF) gb_cmd = 5;  
    }
}


void Ground_Block_Process(void)
{
    if (gb_cmd == 0){
        return;
    }
    uint8_t cmd = gb_cmd;
    gb_cmd = 0;
    switch (cmd)
    {
    case 2: Ground_Block_GetReady();      break;
    case 3: Ground_Block_Fetch(gb_data);  break;
    case 4: Ground_Block_Lay(gb_data);    break;
    case 5: Ground_Block_reset();         break;
    }
}




uint8_t data_test[] = {0};
uint8_t x = 0;


void Ground_Block_Test(){
 
    switch (x)
    {
    case 0: 
      break;
    case 1:
      Ground_Block_GetReady();
      break;
    case 2:
      Ground_Block_Fetch(data_test);
      break;
    case 3:
      Ground_Block_Lay(data_test);
      break;
    case 4:
      Ground_Block_reset();
      break;
    case 5:
      Ground_Block_close();
      break;
    case 6:
      Ground_Block_Enable();
      break;
    }   
}