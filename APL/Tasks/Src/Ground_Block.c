
#include "Ground_Block.h"

uint8_t solenoid_is_open = 0; // 夹爪张开与关闭状态
uint8_t solenoid_enable = 0;  // 夹爪使能状态
volatile uint8_t gb_cmd = 0;
static uint8_t gb_data[8];

 

//使能电机控制，设置为失能模式
void Ground_Block_Enable()
{
    solenoid_enable = 1;
    DJmotor[GROUND_BLOCK_dji_num].Begin = 1;
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Disable;
}

//失能电机控制
void Ground_Block_Disable()
{
    solenoid_enable = 0;                                 // 关闭电磁阀使能
    DJmotor[GROUND_BLOCK_dji_num].Begin = 0;             // 停止电机运行
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Disable; // 设置电机为禁用模式
}

//初始化
void Ground_Block_Init()
{
    Ground_Block_Disable();
}

void Ground_Block_reset()
{
    // 下降高度，打开夹爪
    DJmotor[GROUND_BLOCK_dji_num].MODE_Set = DJ_Position;
    DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = 0.0f;
    solenoid_on(3, 0x07);
    solenoid_is_open = 1;
    osDelay(1000);
}

void Ground_Block_close()
{
    // 先下降高度，再收夹爪,再收主杆

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
    switch (Rxdata[0])
    {
    case 1:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -150.0f;
        osDelay(1500);
        if (solenoid_is_open == 1)
        {
            solenoid_on(3, 0x04); // 夹爪闭合
            solenoid_is_open = 0;
            osDelay(1000);
        }
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -300.0f;
        osDelay(1000);

        break;
    case 2:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -600.0f;
        osDelay(1500);
        if (solenoid_is_open == 1)
        {
            solenoid_on(3, 0x04); // 夹爪闭合
            solenoid_is_open = 0;
            osDelay(1000);
        }
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -750.0f;
        osDelay(1000);
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
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -200.0f;
        osDelay(2000);
        if (solenoid_is_open == 0)
        {
            solenoid_on(3, 0x07); // 夹爪张开
            solenoid_is_open = 1;
            osDelay(1000);
        }
        break;
    case 2:
        DJmotor[GROUND_BLOCK_dji_num].valSet.angle_deg = -650.0f;
        osDelay(2000);
        if (solenoid_is_open == 0)
        {
            solenoid_on(3, 0x07); // 夹爪张开
            solenoid_is_open = 1;
            osDelay(1000);
        }
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
        if (RxHeader.ExtId == 0x01010302)
            gb_cmd = Ground_Block_GetReady_Flag;
        else if (RxHeader.ExtId == 0x01010303)
            gb_cmd = Ground_Block_Fetch_Flag;
        else if (RxHeader.ExtId == 0x01010304)
            gb_cmd = Ground_Block_Lay_Flag;
        else if (RxHeader.ExtId == 0x010103FF)
            gb_cmd = Ground_Block_reset_Flag;
    }
}

void Ground_Block_Process(void)
{
    if (gb_cmd == 0)
    {
        return;
    }
    uint8_t cmd = gb_cmd;
    gb_cmd = 0;
    switch (cmd)
    {
    case Ground_Block_GetReady_Flag:
        Ground_Block_GetReady();
        break;
    case Ground_Block_Fetch_Flag:
        Ground_Block_Fetch(gb_data);
        break;
    case Ground_Block_Lay_Flag:
        Ground_Block_Lay(gb_data);
        break;
    case Ground_Block_reset_Flag:
        Ground_Block_reset();
        break;
    }
}

uint8_t data_test[] = {0};
uint8_t x = 0;

void Ground_Block_Test()
{

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