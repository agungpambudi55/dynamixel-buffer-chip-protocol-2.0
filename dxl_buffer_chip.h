/*
  Copyright (c) 2018 ER2C
  Politeknik Elektronika Negeri Surabaya
  Agung Pambudi <agung.pambudi5595@gmail.com>
  Note :
    Created on Oct 2017
*/

#ifndef dxl_buffer_chip_h
#define dxl_buffer_chip_h

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class DynamixelClass {
  public:
    DynamixelClass(): pinDataControl(-1) { }
    Stream *_serial;                                                  //### Hardware serial RX/TX
    void begin(HardwareSerial &HWserial, long baud);                  //### Calling serial start function
    void end(HardwareSerial &HWserial);                               //### Calling serial end function
    void setDirectionPin(unsigned char pinBufferChip);                //### Set direction control pin for write or read
    void setHoldingTorque(unsigned char ID, bool set);                //### Turn on hold torque
    void setGoalPosition(unsigned char ID, unsigned int pos);         //### Set the goal position for each servo (max. is 4095)
    void setGoalPositionXL320(unsigned char ID, unsigned int pos);    //### Set the goal position for each servo (max. is 1024)
    void setGoalVelocity(unsigned char ID, unsigned int vel);         //### Set the goal velocity for each servo
    void setProfileAcceleration(unsigned char ID, unsigned int pac);  //### Set the profile acc. for each servo (max. is 32767)
    void setProfileVelocity(unsigned char ID, unsigned int pvl);      //### Set the profile velocity for each servo (max. is 1023)
    unsigned int writePacket(unsigned char ID, unsigned short addr, unsigned char *arr, int n);  //### write packet instruction

  private:
    char pinDataControl;                      //### Pin to control TX/RX buffer chip
    unsigned char instructionPacket[64];      //### Array to hold instruction packet data
    unsigned int returnPacket[100];           //### Array to hold returned status packet data
    unsigned int data[15];                    //### Data from return racket  
    unsigned short updateCrc(unsigned char *dataBlkPtr, unsigned short dataBlkSize); //### Update CRC
    void transmitPacket(int transLen);        //### Send packet to dynamixel
    void readReturnPacket(void);              //### Read return packet from dynamixel
    void clearBuffer(void);                   //### Clear RX buffer
};
#endif
