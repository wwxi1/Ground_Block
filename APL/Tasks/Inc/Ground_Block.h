#ifndef GROUND_BLOCK
#define GROUND_BLOCK

#include "solenoid.h"
#include "DJmotor.h"
#include "cmsis_os.h"

#define GROUND_BLOCK_dji_num 1

void Ground_Block_Enable();
void Ground_Block_Disable();
void Ground_Block_Init();
void Ground_Block_reset();
void Ground_Block_reset();
void Ground_Block_GetReady();
void Ground_Block_Fetch(uint8_t *Rxdata);
void Ground_Block_Lay(uint8_t *Rxdata);
void Ground_Block_Func(CAN_RxHeaderTypeDef RxHeader,uint8_t *Rxdata);

#endif

