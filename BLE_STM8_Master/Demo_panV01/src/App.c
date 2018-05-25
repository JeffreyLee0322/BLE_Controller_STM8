//============================================================================//
//  * @file           app.c
//  * @author         Shi Zheng
//  * @version        V1.1.0
//  * @date           Aug/12/2013
//  * @brief          Main program body
//  * @modify user:   Shizheng
//  * @modify date:   Aug/12/2013
//============================================================================//
/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "App.h"
#include "appsub.h"
#include "LED.h"
#include "Key.h"
#include "stdio.h"

extern uint32_t Payload_Count;
extern uint16_t tx_only_count;
extern uint16_t  time_out_count;

unsigned char DISP_Status;
unsigned long int ulState_Timer = 0;
bool DATA_READY = FALSE, TIME_8MS = FALSE;
bool  RX_READY = FALSE,DATA_READY_ADC = FALSE, RX_CRC_CORRECT = FALSE;
bool TX_TRIGGER = FALSE,TX_RQ = FALSE,TX_TIME=FALSE;
bool LED_flag = TRUE;

unsigned char ucLED_FlashCnt = 0;
unsigned short TRX_Cnt = 0;
unsigned char ucTXMode = MODE_TX_SINGLE;

uint8_t status=0;

uint8_t rf_data = 0;

/******************************************************************************/
//            APP_RX_Event
//                RX Event handler
/******************************************************************************/
void APP_RX_Event(void)
{
  if(RX_READY)                                                                  //RF RX data prepare
  {
      RX_READY = FALSE;
      
      //rf_data = RF_Payload.ucPayload[PAYLOAD_WIDTH - 4];
      rf_data = RF_Payload.ucPayload[0];
      
      if (!RX_CRC_CORRECT)
      {
        LED_ChangeFont(0 ,rf_data ,LED_CNT_MODE);
      }
      else
      {
        LED_ChangeFont(1 ,rf_data ,LED_CNT_MODE);
        
      }
  }
  
  if(DATA_READY)                                                                //if Key Event
  {
    DATA_READY = FALSE;
    if(ucKey_RD == KEY1)                                                        //if press Key1 button down
    {
    }
    else if(ucKey_RD == KEY3)                                                 //if press Key3 button down
    {
    }
  }
}
/******************************************************************************/
//            APP_RX_Normal
//                RX StateMachine
/******************************************************************************/

void APP_RX_Normal(void)
{
    APP_RX_Event();                               //Event handler
    Key_Scan();                                   //scan key status
    LED_Display();                                //LED handler
}

//*********************************end of file********************************//