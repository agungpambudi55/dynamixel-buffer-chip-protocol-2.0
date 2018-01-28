/*
  Copyright (c) 2018 ER2C
  Politeknik Elektronika Negeri Surabaya
  Agung Pambudi <agung.pambudi5595@gmail.com>
  Note :
    Created on Oct 2017
*/

#include "dxl_buffer_chip.h"

//############################################################################################################## PUBLIC METHODS #####
void DynamixelClass::begin(HardwareSerial &HWserial, long baud){
  HWserial.begin(baud);   //### Set up serial for a specified serial object
  _serial = &HWserial;
}

void DynamixelClass::end(HardwareSerial &HWserial){ HWserial.end(); }

void DynamixelClass::setDirectionPin(unsigned char pinBufferChip){
  pinDataControl = pinBufferChip;
  pinMode(pinDataControl,OUTPUT);
}

void DynamixelClass::setHoldingTorque(unsigned char ID, bool set) {
  unsigned char arr[1] = {set};
  writePacket(ID, 0x40, arr, 1);
}

void DynamixelClass::setGoalPosition(unsigned char ID, unsigned int pos) {
  pos %= 4096;        //### Limit position
  
  unsigned char arr[] = {
    (pos & 0xFF),
    (pos & 0xFF00) >> 8,
    (pos & 0xFF0000) >> 16,
    (pos & 0xFF000000) >> 24
  };
  
  writePacket(ID, 0x74, arr, 4);
}

void DynamixelClass::setGoalPositionXL320(unsigned char ID, unsigned int pos) {
  pos %= 1024;        //### Limit position
  
  unsigned char arr[] = {
    (pos & 0xFF),
    (pos & 0xFF00) >> 8
  };
  
  writePacket(ID, 0x1E, arr, 2);
}

void DynamixelClass::setProfileAcceleration(unsigned char ID, unsigned int pac) {
  pac %= 32767;      //### Limit acceleration
 
  unsigned char arr[] = {
    (pac & 0xFF),
    (pac & 0xFF00) >> 8,
    (pac & 0xFF0000) >> 16,
    (pac & 0xFF000000) >> 24
  };
  
  writePacket(ID, 0x6C, arr, 4);
}

void DynamixelClass::setProfileVelocity(unsigned char ID, unsigned int pvl) {
  pvl %= 1023;       //### Limit velocity
  
  unsigned char arr[] = {
    (pvl & 0xFF),
    (pvl & 0xFF00) >> 8,
    (pvl & 0xFF0000) >> 16,
    (pvl & 0xFF000000) >> 24
  };
  
  writePacket(ID, 0x70, arr, 4);
}

void DynamixelClass::setGoalVelocity(unsigned char ID, unsigned int vel){
  vel  %= 1023;     //### Limit velocity

  unsigned char arr[] = {
    (vel & 0xFF),
    (vel & 0xFF00) >> 8,
    (vel & 0xFF0000) >> 16,
    (vel & 0xFF000000) >> 24
  };

  writePacket(ID, 0x68, arr, 4);
}

unsigned int DynamixelClass::writePacket(unsigned char ID, unsigned short addr, unsigned char *arr, int n){
  n += 5;

  instructionPacket[0] = ID;                     //### ID
  instructionPacket[1] = (n & 0xFF);             //### Length
  instructionPacket[2] = (n & 0xFF00) >> 8;      //### Length
  instructionPacket[3] = 0x03;                   //### Instruction
  instructionPacket[4] = (addr & 0xFF);          //### Address
  instructionPacket[5] = (addr & 0xFF00) >> 8;   //### Address
  
  for (int i = 0; i < n - 5; i++) { instructionPacket[i+6] = arr[i]; }

  clearBuffer();
  transmitPacket(n);
  
  return 0;
}

void DynamixelClass::readPacket(unsigned char ID, unsigned short addr, int n){
  n += 3;

  instructionPacket[0] = ID;                     //### ID
  instructionPacket[1] = (n & 0xFF);             //### Length of packet
  instructionPacket[2] = (n & 0xFF00) >> 8;      //### Length of packet
  instructionPacket[3] = 0x02;                   //### Instruction
  instructionPacket[4] = (addr & 0xFF);          //### Address
  instructionPacket[5] = (addr & 0xFF00) >> 8;   //### Address
  instructionPacket[6] = ((n-3) & 0xFF);         //### Data length
  instructionPacket[7] = ((n-3) & 0xFF00) >> 8;  //### Data length

  clearBuffer();
  transmitPacket(n);
  readReturnPacket();
}

//############################################################################################################# PRIVATE METHODS #####
void DynamixelClass::transmitPacket(int transmit_length){         //### Transmit instruction packet to Dynamixel
  if (pinDataControl > -1){ digitalWrite(pinDataControl,HIGH); }  //### Set TX Buffer pin to HIGH
  
  int i, delayTransmit = 8;  //### Delay in milliseconds
  unsigned char arr_length = transmit_length+7, pt[arr_length];
  
  pt[0] = 0xFF;
  pt[1] = 0xFF;
  pt[2] = 0xFD;
  pt[3] = 0x00;
  
  for (i = 0; i <= transmit_length; i++) { pt[i+4] = instructionPacket[i]; }
  
  unsigned short crc = updateCrc(pt, arr_length-2);
  unsigned char CRC_L = (crc & 0x00FF);
  unsigned char CRC_H = (crc>>8) & 0x00FF;
  
  i += 4;
  pt[i++] = CRC_L;
  pt[i] = CRC_H;
  
  for(i = 0; i < arr_length; i++) { _serial->write(pt[i]); }
  
//  noInterrupts();

#if defined(__AVR_ATmega32U4__) || defined(__MK20DX128__) || defined(__AVR_ATmega2560__)
  if ((UCSR1A & B01100000) != B01100000){ _serial->flush(); }     //### Wait for TX data to be sent
#elif defined(__SAM3X8E__)
  _serial->flush();
//  Board manager -> Arduino SAM cortex-M3 -> install v1.6.12
//  Solved problem https://github.com/arduino/Arduino/issues/4128
#else
  if ((UCSR0A & B01100000) != B01100000){ _serial->flush(); }     //### Wait for TX data to be sent
#endif

  if (pinDataControl > -1){ digitalWrite(pinDataControl,LOW); }   //### Set TX Buffer pin to LOW after data has been sent
  
//  interrupts();

  delay(20);
}

void DynamixelClass::readReturnPacket(void){
  int i = 0;
  
  while(_serial->available() > 0){        //### Read information when available
    int incomingbyte = _serial->read();   //### Save incomingbyte
    returnPacket[i] = incomingbyte;       //### Save data in return packet array
    i++;
  }
}

void DynamixelClass::clearBuffer(void){  while (_serial->read() != -1); }  //### Clear RX buffer;

unsigned short DynamixelClass::updateCrc(unsigned char *dataBlkPtr, unsigned short dataBlkSize) {
  unsigned short crcAccum = 0; 
  unsigned short crcTable[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
  };

  for(int j = 0; j < dataBlkSize; j++) {
    int i = ((unsigned short)(crcAccum >> 8) ^ dataBlkPtr[j]) & 0xFF;
    crcAccum = (crcAccum << 8) ^ crcTable[i];
  }

  return crcAccum;
}

DynamixelClass Dynamixel;
