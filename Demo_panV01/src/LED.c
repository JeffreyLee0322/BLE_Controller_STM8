//============================================================================//
//  * @file           Sensor.c
//  * @author         Shi Zheng
//  * @version        V1.1.0
//  * @date           Aug/12/2013
//  * @brief          Main program body
//  * @modify user:   Shizheng
//  * @modify date:   Aug/12/2013
//============================================================================//
#include "stm8l15x.h"
#include "LED.h"
#include "App.h"
#include "stdio.h"


static uint8_t ucLED_DIS[4] = {EIGHT,EIGHT,EIGHT,EIGHT};
bool bLED_Enable=TRUE;
/******************************************************************************/
//            LED_TAB
//                LED character Table
/******************************************************************************/
const uint8_t LED_TAB[] = 
{
    0xC0,   //'0'  
    0xF9,   //'1' 
    0xA4,   //'2' 
    0xB0,   //'3' 
    0x99,   //'4' 
    0x92,   //'5' 
    0x82,   //'6' 
    0xF8,   //'7' 
    0x80,   //'8' 
    0x90,   //'9' 
    0xBF,   //'-' 
    0xFF,   //' ' 
    0x86,   //'E' 
    0x88,   //'R' 
    0x46,   //'c.' 
    0x00,   //'8.' 
    0x8E,   //'F'
    0x88,   //'A'
    0x83,   //'b'
    0xc6,   //'C'
    0x0B    //'h'
};
/******************************************************************************/
//            LED_Initial
//                Initial LED Enable pin and CH595 pin
/******************************************************************************/
void LED_Init(void)
{
    unsigned char i;
    for(i=0;i<sizeof(ucLED_DIS);i++)
    {
        ucLED_DIS[i] = 8;
    }
    
    GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast);  
    GPIO_Init(GPIOC, GPIO_Pin_5, GPIO_Mode_Out_PP_High_Fast);  
    GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_Out_PP_High_Fast);
    GPIO_Init(GPIOD, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Fast);
    GPIO_Init(GPIOA, GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast);
    GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Fast);
    GPIO_Init(GPIOD, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Fast);
}

/******************************************************************************/
//            HC595_Write
//                write ucData into HC595
/******************************************************************************/
void HC595_Write(unsigned char ucData)
{
    unsigned char i;
    
    CH595_CP_RESET;
    for(i=0;i<8;i++)
    {
        CH595_CLK_RESET;
        if(ucData&0x80)
        {
            CH595_DAT_SET;        
        }
        else
        {
            CH595_DAT_RESET;        
        }
        ucData<<=1;
        CH595_CLK_SET;
    }
}



/******************************************************************************/
//            LED_Display
//                LED display handler
/******************************************************************************/
void LED_Display(void)
{
    static unsigned char ucLedDis_Bit_Index = 0;

    if(bLED_Enable)                                                             //if Enable LED display
    {
        LED_BIT2_OFF;
        LED_BIT0_OFF;
        LED_BIT1_OFF;
        LED_BIT3_OFF;
        HC595_Write(LED_TAB[ucLED_DIS[ucLedDis_Bit_Index]&0x1F]);               //write data into HC595
        CH595_CP_SET; 
/*******************switch next led**********************/
        if(ucLedDis_Bit_Index == 0)
        {
            LED_BIT0_ON;
            ucLedDis_Bit_Index = 1;
        }
        else if(ucLedDis_Bit_Index == 1)
        {
            LED_BIT1_ON;
            ucLedDis_Bit_Index = 2;
        }
        else if(ucLedDis_Bit_Index == 2)
        {
            LED_BIT2_ON;
            ucLedDis_Bit_Index = 3;
        }
        else if(ucLedDis_Bit_Index == 3)
        {
            LED_BIT3_ON;
            ucLedDis_Bit_Index = 0;
        }
        else
        {
            ucLedDis_Bit_Index = 0;
        }
    }
    else                                                                        //if not Enable LED 
    {
        LED_BIT2_OFF;
        LED_BIT0_OFF;
        LED_BIT1_OFF;
        LED_BIT3_OFF;
        ucLedDis_Bit_Index = 0;      
    }
}

uint8_t get_font(uint8_t input)
{
  switch (input)
  {
  case 10:
    return LED_A;
  case 11:
    return LED_b;
  case 12:
    return LED_C;
  case 13:
    return SUB;
  case 14:
    return LED_E;
  case 15:
    return LED_F;
  default:
    return input;
  }
}

/******************************************************************************/
//            LED_ChangeFont
//                change LED display mode
//                change LED display font
/******************************************************************************/
void LED_ChangeFont(uint8_t correct_flag, uint8_t data, unsigned char ucLED_Mode)
{
  static uint8_t correct_data_old = 0, error_data_old = 0;
 
  if (correct_flag == 1)
  {
    correct_data_old = data;
  }
  else
  {
    error_data_old = data;
  }
  
  switch(ucLED_Mode)
  {
    case LED_NORMAL_MODE:
        ucLED_DIS[3] = SUB;
        ucLED_DIS[2] = SUB;
        ucLED_DIS[1] = SUB;
        ucLED_DIS[0] = SUB;
        break;
    case LED_CNT_MODE:
        ucLED_DIS[3] = get_font(error_data_old / 16);
        ucLED_DIS[2] = get_font(error_data_old % 16);
        ucLED_DIS[1] = get_font(correct_data_old / 16);
        ucLED_DIS[0] = get_font(correct_data_old % 16);
        break;
    default:
        break;

  }
}
      

     


//*******************************end of file*****************************//