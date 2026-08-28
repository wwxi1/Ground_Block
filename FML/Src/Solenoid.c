/*
 * @Author: Frt001 2067314783@qq.com
 * @Date: 2026-08-24 16:51:06
 * @LastEditors: Frt001 2067314783@qq.com
 * @LastEditTime: 2026-08-24 16:51:57
 * @FilePath: \f4_show\FML\Src\Solenoid.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Solenoid.h"

Solenoid_t solenoid_Channel1 = {0};
Solenoid_t solenoid_Channel2 = {0};
Solenoid_t solenoid_Channel3 = {0};

Solenoid_USART_channel_t solenoid_USART_channel ;
/*初始化电磁阀通道，配置其GPIO端口和数据引脚。
solenoid：指向电磁阀结构体的指针，用于存储配置信息
gpio_port：GPIO端口，用于电磁阀控制
gpio_pin_sda：SDA数据引脚编号
gpio_pin_clk：时钟引脚编号*/
static oid solenoid_channel_init(Solenoid_t *solenoid, GPIO_TypeDef *gpio_port,
                           uint16_t gpio_pin_sda, uint16_t gpio_pin_clk)
{
    solenoid->gpio_port = gpio_port;
    solenoid->gpio_pin_sda = gpio_pin_sda;
    solenoid->gpio_pin_clk = gpio_pin_clk;
    solenoid->data_prve = 0xF0;
}

// usart_channel=串口号 不需要在cube中配置 直接调用即可
//初始化指定通道的电磁阀，配置对应的GPIO引脚为推挽输出模式，并关闭电磁阀
void solenoid_init(uint8_t usart_channel)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    switch (usart_channel)
    {
    case Solenoid__USART_one:
        solenoid_channel_init(&solenoid_Channel1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);
        __HAL_RCC_GPIOA_CLK_ENABLE();
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_RESET);
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        break;
    case Solenoid__USART_two:
        solenoid_channel_init(&solenoid_Channel2, GPIOA, GPIO_PIN_2, GPIO_PIN_3);
        __HAL_RCC_GPIOA_CLK_ENABLE();
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        break;
    case Solenoid__USART_three:
        solenoid_channel_init(&solenoid_Channel3, GPIOC, GPIO_PIN_10, GPIO_PIN_11);
        __HAL_RCC_GPIOC_CLK_ENABLE();
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_RESET); /* 将PC10和PC11引脚设置为低电平 */
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11; /* 配置PC10和PC11引脚 */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* 配置为推挽输出模式 */
        GPIO_InitStruct.Pull = GPIO_NOPULL; /* 浮空输入 */
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; /* 配置为中等速度 */
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); /* 初始化GPIOC */
        break;
    default:
        break;
    }
    solenoid_on(usart_channel, 0);
}

/*用于注册并更新电磁阀的数据，通过SPI协议将数据发送到电磁阀设备。
如果新数据与之前的数据相同，则直接返回；否则，通过时钟和数据引脚逐位发送数据。
solenoid：指向Solenoid_t结构体的指针，包含电磁阀的GPIO端口和引脚信息
data：指向uint8_t类型数据的指针，包含要发送的数据*/
void register_updata(Solenoid_t *solenoid, uint8_t *data)
{
    if (*data == solenoid->data_prve) 
    //  检查当前数据是否与上一次发送的数据相同，如果相同则直接返回，避免重复发送
        return;
    solenoid->data_prve = *data;
    for (int i = 0; i < 4; i++)
    {
        if ((*data & 0x08) == 0x08)
        {
            HAL_GPIO_WritePin(solenoid->gpio_port, solenoid->gpio_pin_sda, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(solenoid->gpio_port, solenoid->gpio_pin_sda, GPIO_PIN_RESET);
        }
        *data <<= 1;
        HAL_GPIO_WritePin(solenoid->gpio_port, solenoid->gpio_pin_clk, GPIO_PIN_SET);
        HAL_GPIO_WritePin(solenoid->gpio_port, solenoid->gpio_pin_clk, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(solenoid->gpio_port, solenoid->gpio_pin_clk, GPIO_PIN_SET);
    HAL_GPIO_WritePin(solenoid->gpio_port, solenoid->gpio_pin_clk, GPIO_PIN_RESET);
}

// usart_channel=串口号, cmd=命令(低4位控制) 0000 4321 （4321为对应通道）
void solenoid_on(uint8_t usart_channel, uint8_t cmd)
{
    uint8_t data = cmd & 0x0f;
    switch (usart_channel)
    {
    case 1:
        register_updata(&solenoid_Channel1, &data);
        break;
    case 2:
        register_updata(&solenoid_Channel2, &data);
        break;
    case 3:
        register_updata(&solenoid_Channel3, &data);
        break;
    default:
        break;
    }
}
