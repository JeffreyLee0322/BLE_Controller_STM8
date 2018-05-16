//===================================================================================//
//  * @file           main.c
//  * @author         Shi Zheng
//  * @version        V1.1.0
//  * @date           Aug/12/2013
//  * @brief          Main program body
//  * @modify user:   Cao Zhen
//  * @modify date:   Jan/29/2018
//===================================================================================//
/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "LED.h"
#include "Key.h"
#include "App.h"
#include "appsub.h"
#include "stm8l15x_tim2.h"
#include "stm8l15x_tim3.h"
#include "stm8l15x_dma.h"
#include "stm8l15x_usart.h"
#include "stdio.h"
#include "CPU.H"
#include "VCPU.H"
#include "CONFIG.H"
#include "BIOS.H"

#include "RF.H"
/******************************************************************************/


uint32_t Cycle_times = 0;


/******************************************************************************/
//            main
//            main function body
/******************************************************************************/

void main(void)
{
    BIOS_Init();
#ifdef  DEBUG_EN
    printf("System Initial ok123!  \r\n");
#endif
    //baihua_test();
    while(1)
    {
        //APP_RX_Normal();
        //RFAPI_RXState(TRUE);
        
        RFAPI_TXState();
    }
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/




