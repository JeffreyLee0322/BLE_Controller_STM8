//============================================================================//
//  * @file            RF.c
//  * @author         Shi Zheng 
//  * @version        V1.0
//  * @date           24/4/2015
//  * @brief          RFPN006 communication interface
//  * @modify user:   Shizheng
//  * @modify date:   24/4/2015
//============================================================================//
#include "RF.H"


//const uint8_t TX_ADDRESS_DEF[4] = {0x71, 0x91, 0x7D, 0x6B};//{0x71, 0x91, 0x7D, 0x6B};
const uint8_t TX_ADDRESS_DEF[4] = {0x71, 0x91, 0x7D, 0x6B};//{0x71, 0x91, 0x7D, 0x6B};
uint8_t TxPayloadLength = PAYLOAD_WIDTH;
const uint8_t AckPayloadLength = 0x00;
uint32_t Payload_Count = 0;
uint16_t tx_only_count=0;
uint16_t  time_out_count=0;

uint32_t rx_times = 0;

/******************************************************************************/
//            SPI_init
//               init spi pin and IRQ  CE input/out mode
/******************************************************************************/
void SPI_init(void)
{
    GPIO_Init( GPIOD, GPIO_Pin_0, GPIO_Mode_In_PU_No_IT);                       //IRQ  input pulling high without interrupt
    GPIO_Init( GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Fast);                   //CE   output Low pulling push    

    GPIO_Init( GPIOB, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast);                  //CSN  output High pulling push
    GPIO_Init( GPIOB, GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast);                   //SCK  output Low  pulling push 
    GPIO_Init( GPIOB, GPIO_Pin_6, GPIO_Mode_Out_PP_High_Fast);                  //MOSI output High pulling push
    GPIO_Init( GPIOB, GPIO_Pin_7, GPIO_Mode_In_PU_No_IT);                       //MISO input pull high
}


/******************************************************************************/
//            SPI_RW
//                SPI Write/Read Data
//            SPI写入一个BYTE的同时，读出一个BYTE返回
/******************************************************************************/
 uint8_t SPI_RW( uint8_t	 R_REG)
{
    uint8_t	  i;
    for(i = 0; i < 8; i++)
    {
        SCK_LOW;
        if(R_REG & 0x80)
        {
            MOSI_HIGH;
        }
        else
        {
            MOSI_LOW;
        }
        R_REG = R_REG << 1;
        SCK_HIGH;
        if( MISO_STATUS )
        {
          R_REG = R_REG | 0x01;
        }
    }
    SCK_LOW;
    return R_REG;
}

/******************************************************************************/
//            RF_WriteReg
//                Write Data(1 Byte Address ,1 byte data)
/******************************************************************************/
void RF_WriteReg( uint8_t reg,  uint8_t wdata)
{
    CSN_LOW;
    SPI_RW(reg);
    SPI_RW(wdata);
    CSN_HIGH;
}


/******************************************************************************/
//            RF_ReadReg
//                Read Data(1 Byte Address ,1 byte data return)
/******************************************************************************/
 uint8_t  ucRF_ReadReg( uint8_t reg)
{
     uint8_t tmp;
    
    CSN_LOW;
    SPI_RW(reg);
    tmp = SPI_RW(0);
    CSN_HIGH;
    
    return tmp;
}

/******************************************************************************/
//            RF_WriteBuf
//                Write Buffer
/******************************************************************************/
void RF_WriteBuf( uint8_t reg, uint8_t *pBuf, uint8_t length)
{
     uint8_t j;
    CSN_LOW;
    j = 0;
    SPI_RW(reg);
    for(j = 0;j < length; j++)
    {
        SPI_RW(pBuf[j]);
    }
    j = 0;
    CSN_HIGH;
}

/******************************************************************************/
//            RF_ReadBuf
//                Read Data(1 Byte Address ,length byte data read)
/******************************************************************************/
void RF_ReadBuf( uint8_t reg, unsigned char *pBuf,  uint8_t length)
{
    uint8_t byte_ctr;

    CSN_LOW;                    		                               			
    SPI_RW(reg);       		                                                		
    for(byte_ctr=0;byte_ctr<length;byte_ctr++)
    	pBuf[byte_ctr] = SPI_RW(0);                                                 		
    CSN_HIGH;                                                                   		
}



/******************************************************************************/
//            RF_TxMode
//                Set RF into TX mode
/******************************************************************************/
void RF_TxMode(void)
{
    
    RF_WriteReg(W_REGISTER + CONFIG,  0X8E);							// 将RF设置成TX模式
    delay_ms(10);   
    CE_HIGH;	
    delay_ms(10);
}


/******************************************************************************/
//            RF_RxMode
//            将RF设置成RX模式，准备接收数据
/******************************************************************************/
void RF_RxMode(void)
{
    RF_WriteReg(W_REGISTER + CONFIG,  0X87 );							// 将RF设置成RX模式   			                                       
    delay_ms(10);   
    CE_HIGH;	
    delay_ms(10);
}

/******************************************************************************/
//            RF_GetStatus
//            read RF IRQ status,3bits return
/******************************************************************************/
uint8_t ucRF_GetStatus(void)
{
    return ucRF_ReadReg(STATUS)&0x70;								//读取RF的状态 
}

/******************************************************************************/
//            RF_ClearStatus
//                clear RF IRQ
/******************************************************************************/
void RF_ClearStatus(void)
{
    RF_WriteReg(W_REGISTER + STATUS,0x70);							//清除RF的IRQ标志 
}

/******************************************************************************/
//            RF_ClearFIFO
//                clear RF TX/RX FIFO
/******************************************************************************/
void RF_ClearFIFO(void)
{ 
    RF_WriteReg(FLUSH_TX, 0);			                                		//清除RF 的 TX FIFO		
    RF_WriteReg(FLUSH_RX, 0);                                                   		//清除RF 的 RX FIFO	
}

/******************************************************************************/
//            RF_SetChannel
//                Set RF TX/RX channel:Channel
/******************************************************************************/
void RF_SetChannel( uint8_t Channel)
{
    CE_LOW;
    RF_WriteReg(W_REGISTER + RF_CH, Channel);
}

/******************************************************************************/
//            发送数据：
//            参数：
//              1. ucPayload：需要发送的数据首地址
//              2. length:  需要发送的数据长度
//              Return:
//              1. MAX_RT: TX Failure  (Enhance mode)
//              2. TX_DS:  TX Successful (Enhance mode)
//              note: Only use in Tx Mode
//              length 通常等于 PAYLOAD_WIDTH
/******************************************************************************/
void ucRF_TxData( uint8_t *ucPayload,  uint8_t length)
{
    RF_WriteBuf(W_TX_PAYLOAD, ucPayload, length);                               		//write data to txfifo                                                                      		//rf entery tx mode start send data                                                                       		//rf entery stb3
    delay_ms(20);
}

/******************************************************************************/
//            发送数据：没有外部IRQ
//            参数：
//              1. ucPayload：需要发送的数据首地址
//              2. length:  需要发送的数据长度
//              Return:
//              1. MAX_RT: TX Failure  (Enhance mode)
//              2. TX_DS:  TX Successful (Enhance mode)
//              note: Only use in Tx Mode
//              length 通常等于 PAYLOAD_WIDTH
/******************************************************************************
uint8_t ucRF_TxData( uint8_t *ucPayload,  uint8_t length)
{
   

   if(0==ucRF_GetStatus())  
   {
    RF_WriteBuf(W_TX_PAYLOAD, ucPayload, length);                               		//write data to txfifo        
  
    CE_HIGH;                                                                    		//rf entery tx mode start send data 
    delay_10us(2);                                                              		//keep ce high at least 16us
    CE_LOW;                                                                     		//rf entery stb3
   }
   
  if((ucRF_GetStatus()&&TX_DS_FLAG))
    {     
      RF_ClearFIFO();
      RF_ClearStatus ();                              		
    }     
  
} 
*/
/******************************************************************************/
//            ucRF_DumpRxData
//            读出接收到的数据：
//            参数：
//              1. ucPayload：存储读取到的数据的Buffer
//              2. length:    读取的数据长度
//              Return:
//              1. 0: 没有接收到数据
//              2. 1: 读取接收到的数据成功
//              note: Only use in Rx Mode
//              length 通常等于 PAYLOAD_WIDTH
/******************************************************************************/
uint8_t ucRF_DumpRxData( uint8_t *ucPayload,  uint8_t length)
{ 
    if(!(ucRF_GetStatus()&&RX_DR_FLAG))
    {
      return 0;                                                                 		
    }
    
    rx_times++;
    if(rx_times > 0xfffff0)
    {
        rx_times = 0;
    }
    
    RF_ReadBuf(R_RX_PAYLOAD, ucPayload, length);
 
    
     return 1;
}



/******************************************************************************/
//            发送结果
//            参数：只在增强模式下，使能ack带Payload有效
//                  1、ucAckPayload: AckPayload的首地址
//                  2、length：AckPayload的长度
/******************************************************************************/

void RF_TX_CheckResult(uint8_t *ucAckPayload, uint8_t length)
{
  switch(ucRF_GetStatus())
  {
  case TX_DS_FLAG: 
         tx_only_count++;
         RF_ClearFIFO();
         RF_ClearStatus();
         break;
           
  case RX_TX_FLAG:
         RF_ReadBuf(R_RX_PAYLOAD,ucAckPayload,length);
         ++Payload_Count;
         RF_ClearFIFO();
         RF_ClearStatus();
         break;
    
  case MAX_RT_FLAG:
         time_out_count++;
         RF_ClearFIFO();
         RF_ClearStatus();
         break;
  default:
         break;
  }
}

////////////////////////////////////////////////////////////////////////////////

//          以下部分与RF通信相关，不建议修改
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
//            PN006_Initial
//                Initial RF
/******************************************************************************/
void RF_Init(void)
{
  
  uint8_t BB_cal_data[]	  = {0x12,0xED,0x67,0x9C,0x46};
  uint8_t RF_cal_data[]	  = {0xF6,0x3F,0x5D};
  uint8_t RF_cal2_data[]  = {0x45,0x21,0xEF,0x2C,0x5A,0x40};
  uint8_t Dem_cal_data[]  = {0x00};
  //uint8_t Dem_cal2_data[] = {0x0B,0x0F,0x02};//16:13 = 0000 2bytes ---> 16bit
  uint8_t Dem_cal2_data[] = {0x0B,0x0F,0x03};//16:13 = 1000 3bytes --->  24bit
  //uint8_t Dem_cal2_data[] = {0x0B,0xCF,0x02};//16:13 = 0110 3bytes  ----> 22bit

    uint8_t feature = 0x00;
    SPI_init();

    RF_WriteReg(RST_FSPI, 0x5A);								//Software Reset    			
    RF_WriteReg(RST_FSPI, 0XA5);
    
    RF_WriteReg(FLUSH_TX, 0);									// CLEAR TXFIFO		    			 
    RF_WriteReg(FLUSH_RX, 0);									// CLEAR  RXFIFO
    RF_WriteReg(W_REGISTER + STATUS, 0x70);							// CLEAR  STATUS	
    RF_WriteReg(W_REGISTER + EN_RXADDR, 0x01);							// Enable Pipe0
    RF_WriteReg(W_REGISTER + SETUP_AW,  0x02);							// address witdth is 5 bytes
    RF_WriteReg(W_REGISTER + RF_CH,     DEFAULT_CHANNEL);//DEFAULT_CHANNEL                                       // 2478M HZ
    RF_WriteReg(W_REGISTER + RX_PW_P0,  PAYLOAD_WIDTH);						// 8 bytes
    RF_WriteBuf(W_REGISTER + TX_ADDR,   ( uint8_t*)TX_ADDRESS_DEF, sizeof(TX_ADDRESS_DEF));	// Writes TX_Address to PN006
    RF_WriteBuf(W_REGISTER + RX_ADDR_P0,( uint8_t*)TX_ADDRESS_DEF, sizeof(TX_ADDRESS_DEF));	// RX_Addr0 same as TX_Adr for Auto.Ack   
    RF_WriteBuf(W_REGISTER + BB_CAL,    BB_cal_data,  sizeof(BB_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL2,   RF_cal2_data, sizeof(RF_cal2_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL,   Dem_cal_data, sizeof(Dem_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL,    RF_cal_data,  sizeof(RF_cal_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL2,  Dem_cal2_data,sizeof(Dem_cal2_data));
    RF_WriteReg(W_REGISTER + DYNPD, 0x00);					                            
    RF_WriteReg(W_REGISTER + RF_SETUP,  RF_POWER);						// 13DBM  		
    RF_WriteReg(ACTIVATE, 0x73);
    
#if(TRANSMIT_TYPE == TRANS_ENHANCE_MODE)      
    RF_WriteReg(W_REGISTER + SETUP_RETR,0x01);							//  1 retrans... 	
    RF_WriteReg(W_REGISTER + EN_AA,     0x01);							// Enable Auto.Ack:Pipe0  	
#elif(TRANSMIT_TYPE == TRANS_BURST_MODE)                                                                
    RF_WriteReg(W_REGISTER + SETUP_RETR,0x00);							// Disable retrans... 	
    RF_WriteReg(W_REGISTER + EN_AA,     0x00);							// Disable AutoAck 
#endif

#if(EN_DYNPLOAD == 1)
    feature |= 0x04;
    RF_WriteReg(W_REGISTER + DYNPD, 0x01);
#endif

#if(EN_ACK_PAYLOAD == 1)
    feature |= 0x02;
#endif
 
if(PAYLOAD_WIDTH >32)
    feature |= 0x18;

RF_WriteReg(W_REGISTER + FEATURE, feature);

}

/******************************************************************************/
//            		进入载波模式
/******************************************************************************/
void RF_Carrier( uint8_t ucChannel_Set)
{
    uint8_t BB_cal_data[]    = {0x0A,0x6D,0x67,0x9C,0x46}; 
    uint8_t RF_cal_data[]    = {0xF6,0x37,0x5D};
    uint8_t RF_cal2_data[]   = {0x45,0x21,0xEF,0xAC,0x5a,0x50};
    uint8_t Dem_cal_data[]   = {0xE1}; 								
    uint8_t Dem_cal2_data[]  = {0x0B,0xDF,0x02};      

    CE_LOW;
    delay_10us(500);				 						//delay 500us
    RF_WriteReg(W_REGISTER + CONFIG, 0X8e);        						//tx mode 
    RF_WriteReg(W_REGISTER + RF_CH, ucChannel_Set);						//单载波频点	   
    RF_WriteReg(W_REGISTER + RF_SETUP, RF_POWER);      						//13dbm
    RF_WriteBuf(W_REGISTER + BB_CAL,    BB_cal_data,  sizeof(BB_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL2,   RF_cal2_data, sizeof(RF_cal2_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL,   Dem_cal_data, sizeof(Dem_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL,    RF_cal_data,  sizeof(RF_cal_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL2,  Dem_cal2_data,sizeof(Dem_cal2_data));
    delay_10us(500);	
}

/***************************************end of file ************************************/
