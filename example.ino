/*
  Created on Jan 2018
  Agung Pambudi <agung.pambudi5595@gmail.com>
*/

# Example for how to use this library

#include "dxl_buffer_chip.h"

#define ID            0x01  // ID Dynamixel
#define ControlPin    0x08  // Control pin of buffer chip

// degree / goal position
int i=0, pos[]={0, 1023, 2047, 3071, 4095};

DynamixelClass Dxl;

void setup(){
  Serial.begin(115200);  
  Dxl.begin(Serial2,115200);
  Dxl.setDirectionPin(ControlPin);
  Dxl.setHoldingTorque(IDMX, true);
  Dxl.setProfileVelocity(IDMX, 200);
  Dxl.setProfileAcceleration(IDMX, 20);
}

void loop(){
  Dxl.setGoalPosition(IDMX, pos[i]);
  delay(2500);
  Serial.println(Dxl.getLoad(IDMX)); 
  Serial.println(Dxl.getPositionSteps(IDMX)); 
  Serial.println(Dxl.getPositionDegrees(IDMX));
  Serial.println(); 
  i++;
  if(i>5)i=0;
}
