#ifndef GROUND_BLOCK
#define GROUND_BLOCK

#include "solenoid.h"
#include "DJmotor.h"
#include "cmsis_os.h"

#define GROUND_BLOCK_dji_num 5

#define Ground_Block_GetReady_Flag 2
#define Ground_Block_Fetch_Flag 3
#define Ground_Block_Lay_Flag 4
#define Ground_Block_reset_Flag 5

void Ground_Block_Enable(void);
void Ground_Block_Disable(void);
void Ground_Block_Init(void);
void Ground_Block_reset(void);
void Ground_Block_GetReady(void);
void Ground_Block_Fetch(uint8_t *Rxdata);
void Ground_Block_Lay(uint8_t *Rxdata);
void Ground_Block_Func(CAN_RxHeaderTypeDef RxHeader, uint8_t *Rxdata);
void Ground_Block_Process(void);



#endif

