#include <Wire.h>

float numbers[3];

void setup() {
  Wire.begin(); // Join I2C bus as master
  Serial.begin(115200); // Start the serial communication
  delay(1000); // Give some time for the receiver to set up
}

void loop() {
  numbers[0]= 0.01 + random(5, 25);
  numbers[1]=int(0.01 + random(228, 235));
  numbers[2]= 0.01 + random(1, 13);
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
