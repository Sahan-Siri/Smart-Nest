#include <Wire.h>
#include "EmonLib.h"
#include <math.h>

float numbers[3];
EnergyMonitor emon1; 

void setup() {
  Wire.begin(); // Join I2C bus as master
  Serial.begin(9600); // Start the serial communication
  emon1.voltage(A1, 234.26, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(A0,10.005);       // Current: input pin, calibration.
  delay(1000); // Give some time for the receiver to set up
}

void loop() {
  emon1.calcVI(20,2000);
  float realPower       = emon1.realPower;        //extract Real Power into variable
  //float apparentPower   = emon1.apparentPower;    //extract Apparent Power into variable
  //float powerFActor     = emon1.powerFactor;      //extract Power Factor into Variable
  float supplyVoltage   = emon1.Vrms;             //extract Vrms into Variable
  float Irms            = emon1.Irms; 
  Serial.print("Voltage:");            //extract Irms into Variable
  Serial.println(supplyVoltage);
  Serial.print("Current:");
  Serial.println(Irms);
  Serial.print("Power:");
  Serial.println(realPower);
  if (supplyVoltage<=10){
    supplyVoltage=0.00;
  }
  if (Irms<=0.11){
    Irms=0.00;
  }
  numbers[0] = roundf(realPower * 100) / 100000.0;
  numbers[1] = roundf(supplyVoltage * 100) / 100.0;
  numbers[2] = roundf(Irms * 100) / 100.0;
  
  sendFloats(numbers, 3);
  delay(1000); // Add a delay to avoid spamming the I2C bus
}

void sendFloats(float* nums, int count) {
  Wire.beginTransmission(8); // Transmit to device #8

  for (int i = 0; i < count; i++) {
    byte *byteData = (byte *) &nums[i]; // Cast float to byte array
    for (int j = 0; j < sizeof(float); j++) {
      Wire.write(byteData[j]);
    }
  }

  Wire.endTransmission(); // Stop transmitting
  
  // Print the sent numbers for debugging
  for (int i = 0; i < count; i++) {
    Serial.println("Number sent: " + String(nums[i]));
  }
}
