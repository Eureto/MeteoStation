#include <Arduino.h>
#include <HardwareSerial.h>
#include <queue>
#include <Wire.h>
#include <vector>
#include "ble_setup.h"
#include "AHT20_BMP280_CONTROL.h"
#include "internal_temp.h"

//  Heartbeat
unsigned long HeartbeatMillis = 0;
const long Heartbeatinterval = 5000;

float delta = 0;
float minDelta = 10;
float maxDelta = 0;


std::vector<float> temperature_vector;
std::vector<float> pressure_vector;
std::vector<float> humidity_vector;



// void list(){
//   std::string response = "";
//   for (int i = 0; i < temperature_vector.size(); i++) {
//     response += "Temp: " + std::to_string(temperature_vector[i]) + "  ";
//     response += "Pres: " + std::to_string(pressure_vector[i]) + "  ";
//     response += "Hum: " + std::to_string(humidity_vector[i]) + " \n";
//   }
// }



void setup() {
  Serial.begin(115200);
  delay(1000);  // Give time for serial to initialize
  
  Serial.println("Starting MeteoStation...");
  
  Wire.begin();
  
  // Initialize sensors first
  Serial.println("Initializing sensors...");
  AHT20_begin();
  BMP280_begin();
  startMeasurementAHT20();
  
  // Initialize BLE last
  Serial.println("Initializing BLE...");
  initBLE();
  
  Serial.println("Setup complete!");
}

void loop() {


  checkbusyAHT20();
  getDataAHT20();

  unsigned long currentMillis = millis();
  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;

  
    //BMP280
    readTemperatureBMP280();
    Serial.print("Temperatur: ");
    Serial.print(temperature_BMP280);
    Serial.println(" C");
    temperature_vector.push_back(temperature_AHT20);

    readPressureBMP280();
    pressure_vector.push_back(pressure);
    Serial.print("Druck: ");
    Serial.print(pressure);
    Serial.println(" hPa");
 

    // AHT20
    startMeasurementAHT20();
    humidity_vector.push_back(humidity);
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Temperatur: ");
    Serial.print(temperature_AHT20);
    Serial.println(" C");



    // Calculate the delta between the temperature values AHT20 and BMP280
    delta = (temperature_BMP280 > temperature_AHT20) ? (temperature_BMP280 - temperature_AHT20) : (temperature_AHT20 - temperature_BMP280);

    if (delta < minDelta) {
      minDelta = delta;
    }
    if (delta > maxDelta) {
      maxDelta = delta;
    }
    Serial.print("Temperatur Delta : ");
    Serial.print(delta);
    Serial.print(" | Min Delta: ");
    Serial.print(minDelta);
    Serial.print(" | Max Delta: ");
    Serial.println(maxDelta);
    
    delay(5000); // Small delay to ensure stability
    // Update BLE characteristics with new sensor data
    updateBLEData();
  }
}

