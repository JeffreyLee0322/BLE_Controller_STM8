//============================================================================//
//  * @file           RFAPI.c
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
#include "APPSub.h"
#include "LED.h"
#include "Key.h"
#include "stdio.h"

#include "adv_format.h"

bool  bRFAPI_IRQ = FALSE;
RF_PAYLOAD  RF_Payload;

uint8_t whitening_reg[7] = {0, };
uint8_t crc_reg[24] = {0, };

uint32_t Correct_Data = 0, Wrong_Data = 0, Have_Data = 0;
uint8_t Payload_Data_Pre_Crc[16] = {0};
int recv_data = 0;
int recv_data1 = 0;
int recv_data2[16] = 0;
int recv_data3[43] = 0;
int recv_data4 = 0;
uint8_t result_test = 0;

uint8 ADV_Format_Data[50] = {0};

void whitening_init(uint8_t channel_index)
{
	whitening_reg[0] = 1;
	
	for (uint8_t i = 1; i < 7; i++)
	{
		whitening_reg[i] = (channel_index >> (6 - i)) & 0x01;
	}
}

uint8_t whitening_output(void)
{
	uint8_t temp = whitening_reg[3] ^ whitening_reg[6];
	
	whitening_reg[3] = whitening_reg[2];
	whitening_reg[2] = whitening_reg[1];
	whitening_reg[1] = whitening_reg[0];
	whitening_reg[0] = whitening_reg[6];
	whitening_reg[6] = whitening_reg[5];
	whitening_reg[5] = whitening_reg[4];
	whitening_reg[4] = temp;
	
	return whitening_reg[0];
}

void whitening_decode(uint8_t *data, uint8_t length)
{
	for (uint8_t data_index = 0; data_index < length; data_index++)
	{
		uint8_t data_input = data[data_index];
		uint8_t data_bit = 0;
		uint8_t data_output = 0;
		
		for (uint8_t bit_index = 0; bit_index < 8; bit_index++)
		{
			data_bit = (data_input >> (bit_index)) & 0x01;
			
			data_bit ^= whitening_output();
			
			data_output += (data_bit << (bit_index));
		}
		
		data[data_index] = data_output;
	}
}

void crc_init(void)
{
	uint32_t crc = 0x00555555;
	
	for (uint8_t i = 0; i < 24; i++)
	{
		crc_reg[i] = (crc >> i) & 0x00000001;
	}
}

uint32_t arr_to_crc(void)
{
	uint32_t output_data = 0;
	
	for (uint8_t i = 0; i < 24; i++)
	{
		output_data |= (crc_reg[i] << (23 - i));
	}
	
	return output_data;
}

void crc_update(uint8_t input_data_bit)
{

	uint8_t temp = crc_reg[23] ^ input_data_bit;
	
	crc_reg[23] = crc_reg[22];
	crc_reg[22] = crc_reg[21];
	crc_reg[21] = crc_reg[20];
	crc_reg[20] = crc_reg[19];
	crc_reg[19] = crc_reg[18];
	crc_reg[18] = crc_reg[17];
	crc_reg[17] = crc_reg[16];
	crc_reg[16] = crc_reg[15];
	crc_reg[15] = crc_reg[14];
	crc_reg[14] = crc_reg[13];
	crc_reg[13] = crc_reg[12];
	crc_reg[12] = crc_reg[11];
	crc_reg[11] = crc_reg[10];
	crc_reg[10] = crc_reg[9] ^ temp;
	crc_reg[9] = crc_reg[8] ^ temp;
	crc_reg[8] = crc_reg[7];
	crc_reg[7] = crc_reg[6];
	crc_reg[6] = crc_reg[5] ^ temp;
	crc_reg[5] = crc_reg[4];
	crc_reg[4] = crc_reg[3] ^ temp;
	crc_reg[3] = crc_reg[2] ^ temp;
	crc_reg[2] = crc_reg[1];
	crc_reg[1] = crc_reg[0] ^ temp;
	crc_reg[0] = temp;
}

void check_crc(uint8_t *data, uint8_t length)
{
	for (uint8_t data_index = 0; data_index < length; data_index++)
	{
		uint8_t data_input = data[data_index];
		uint8_t data_bit = 0;
		
		for (uint8_t bit_index = 0; bit_index < 8; bit_index++)
		{
			data_bit = (data_input >> (bit_index)) & 0x01;
			crc_update(data_bit);
		}
	}
}

void baihua_test()
{
  uint8_t data[1] = {225};
  
  whitening_init(37);
  whitening_decode(data, 1);
  
  result_test = data[0];
}

/******************************************************************************/
//            BB_RX_State
//                In RX status ,Judge data has been received and read them
/******************************************************************************/
void  RFAPI_RXState(bool RX_Continue)
{
    uint8_t i = 0;
    uint8_t pointer = 0;
    if(ucRF_DumpRxData(RF_Payload.ucPayload,PAYLOAD_WIDTH))//16Bytes
    {
        RF_ClearFIFO();
        RF_ClearStatus();
        
        recv_data = RF_Payload.ucPayload[0];
        //if(0x25 == recv_data)
        {
          RX_CRC_CORRECT = TRUE;
          RX_READY = TRUE;
          //while(1);
          //return ack
        }
    }
}

void  RFAPI_TXState(void)
{
    adv_init_format_get(ADV_Format_Data);
    
    whitening_init(37);
    whitening_decode(ADV_Format_Data+5, 13-5);
    
    ucRF_TxData(ADV_Format_Data, 13);
    if(ucRF_GetStatus()==TX_DS_FLAG)                //Check  the register sent successfully
    {
        RF_ClearFIFO();                            //clear fifo and status
        RF_ClearStatus();
    }
}



//********************************end of file*********************************//