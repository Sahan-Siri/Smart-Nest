const int sensorPin = A0;  // Pin connected to the sensor's output
const float VCC = 5.0;     // Supply voltage to ACS712
const float sensorZeroCurrent = 0.614;  // Zero current output is half of VCC
const int numSamples = 1000;  // Number of samples to take for RMS calculation

void setup() {
  Serial.begin(9600);
}

void loop() {
  float sumCurrentSquared = 0;
  float sumVoltageSquared = 0;

  // Take multiple samples to calculate RMS
  for (int i = 0; i < numSamples; i++) {
    int sensorValue = analogRead(sensorPin);
    float voltage = (sensorValue / 4096.0) * VCC;
    float current = (voltage - sensorZeroCurrent) / 0.1;  // Sensitivity is 100mV per Ampere for ACS712-20A

    sumVoltageSquared += voltage * voltage;
    sumCurrentSquared += current * current;

    delayMicroseconds(1000);  // Sampling delay
  }

  // Calculate RMS values
  float rmsVoltage = sqrt(sumVoltageSquared / numSamples);
  float rmsCurrent = sqrt(sumCurrentSquared / numSamples);

  // Print the RMS voltage and current
  Serial.print("RMS Voltage: ");
  Serial.print(rmsVoltage, 3);
  Serial.print(" V, RMS Current: ");
  Serial.print(rmsCurrent, 3);
  Serial.println(" A");

  delay(1000);  // Update every second
}
