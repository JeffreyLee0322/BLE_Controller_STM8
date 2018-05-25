#include "adv_format.h"

typedef struct adv_header
{
  uint8 ADV_PDU_Type:4;
  uint8 ADV_RFU:2;
  uint8 ADV_TxAdd:1;
  uint8 ADV_RxAdd:1;
  uint8 ADV_Length:6;
  uint8 ADV_RFU_:2;
}ADV_PDU_HEADER;

typedef struct adv_ind_pdu
{
  uint8 ADV_MAC_Address[6];
  uint8 ADV_PDU_Data[31];
}ADV_IND_PDU_DATA;

uint8 mac_address[6] = {0xA8, 0x8A, 0x5C, 0x76, 0xA4, 0x63};//HUAWEI P9: 0x67A4765C8AA8
uint8 access_address[4] = {0xD6, 0xBE, 0x89, 0x8E};//0x8E89BED6
//uint8 adv_format[50];

void adv_init_format_get(uint8 *adv_format)
{
  ADV_PDU_HEADER adv_pdu_header;
  ADV_IND_PDU_DATA adv_ind_pdu_data;
  
  adv_pdu_header.ADV_PDU_Type = 0x00; //ADV_IND
  adv_pdu_header.ADV_RFU = 0x00;
  adv_pdu_header.ADV_TxAdd = 0x01;
  adv_pdu_header.ADV_RxAdd = 0x00;
  adv_pdu_header.ADV_Length = 0x06;//6-37 0ctets
  adv_pdu_header.ADV_RFU_ = 0x00;
  
  memcpy(adv_ind_pdu_data.ADV_MAC_Address, mac_address, 6);//Set ADV Mac address
  memset(adv_ind_pdu_data.ADV_PDU_Data, 0, 31);
  
  //Packet
  adv_format[0] = 0xAA; //Preamble
  memcpy(adv_format+1, access_address, 4);
  memcpy(adv_format+5, &adv_pdu_header, 2);
  memcpy(adv_format+7, &adv_ind_pdu_data, 37);
}

