
#include "Ground_Block.h"

uint8_t solenoid_is_open = 0; // 夹爪张开与关闭状态
uint8_t solenoid_enable = 0; // 夹爪使能状态

void Ground_Block_Enable()
{
    solenoid_enable = 1;
    DJmotor[0].Begin = 1;
    DJmotor[0].MODE_Set = DJ_Position;

}
void Ground_Block_Disable()
{
    solenoid_enable = 0;        // 关闭电磁阀使能
    DJmotor[0].Begin = 0;       // 停止电机运行
    DJmotor[0].MODE_Set = DJ_Disable; // 设置电机为禁用模式

}

void Ground_Block_Init()
{
    void Ground_Block_Disable();
}


/*
void solenoid_reset()
{
    uint8_t data1 = 0x04; //  先收夹爪
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
    DJmotor[0].valSet.angle_deg = 0.0f;
    solenoid_on(2, 0x01);
    solenoid_on(3, 0x01);
    solenoid_is_open = 1;
    osDelay(1000);
}

void Ground_Block_close()
{
   //先下降高度，再收夹爪,再收主杆
    DJmotor[0].valSet.angle_deg = 0.0f;
    osDelay(4000);
    solenoid_on(2, 0x00);
    solenoid_on(3, 0x00);
    solenoid_is_open = 0;
    solenoid_on(1, 0x00);
    osDelay(1000);
}

void Ground_Block_GetReady() // 预取大地块
{
    // 杆推出，夹爪张开
    solenoid_on(1, 0x01);
    solenoid_on(2, 0x01);
    solenoid_on(3, 0x01);
    solenoid_is_open = 1;
}

// 接收数据 并执行对应动作，此处osdelay需更改，目前测试用
void Ground_Block_Fetch(uint8_t *Rxdata)
{
     if (solenoid_is_open = 1)
        {
            solenoid_on(2, 0x00); // 夹爪闭合
            solenoid_on(3, 0x00);
            osDelay(1000);
        }

    switch (Rxdata[0])
    {
    case 1:

         DJmotor[0].valSet.angle_deg = 0.0f;

        break;
    case 2:
        
         DJmotor[0].valSet.angle_deg = 0.0f;

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
        DJmotor[0].valSet.angle_deg = 0.0f;
        osDelay(2000);
        solenoid_on(2, 0x01); // 夹爪张开
        solenoid_on(3, 0x01);
        break;
    case 2:
        DJmotor[0].valSet.angle_deg = 0.0f;
        osDelay(2000);
        solenoid_on(2, 0x01); // 夹爪张开
        solenoid_on(3, 0x01);
        osDelay(1000);
        break;
    default:
        break;
    }
}

void Ground_Block_Func(CAN_RxHeaderTypeDef RxHeader,uint8_t *Rxdata)
{
    if (solenoid_is_open == 1 && DJmotor[0].Begin == 1)
    {
        if(RxHeader.ExtId == 0x01010303)
        {
            Ground_Block_Fetch(Rxdata);
        }
        else if (RxHeader.ExtId == 0x01010304)
        {
            Ground_Block_Lay(Rxdata);
        }
        else if (RxHeader.ExtId == 0x01010302)
        {
            Ground_Block_GetReady();
        }
        else if (RxHeader.ExtId == 0x010103FF)
        {
           Ground_Block_reset();
        }
        else if (RxHeader.ExtId == 0x01010301)
        {
           Ground_Block_Enable();
        }
        else 
        {
            return;
        }

}
}