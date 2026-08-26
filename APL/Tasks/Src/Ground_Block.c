
#include "Ground_Block.h"

uint8_t solenoid_signal = 0;

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

// 接收数据 并执行对应动作，此处osdelay需更改，目前测试用
void solenoid_receive(uint8_t *Rxdata)
{
    switch (Rxdata[0])
    {
    case 0:
        if (solenoid_signal = 1)
        {
            solenoid_reset();
            solenoid_signal = 0;
        }
        break;
    case 1:
        if (solenoid_signal = 0)
        {
            solenoid_on(3, 0x04);
            osDelay(3000);
            solenoid_signal = 1;
        }
        switch (Rxdata[1])
        {
        case 0:
            solenoid_on(1, 0x04); // 夹爪张开
            osDelay(3000);
            break;
        case 1:
            solenoid_on(3, 0x07); // 夹爪闭合
            osDelay(3000);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}