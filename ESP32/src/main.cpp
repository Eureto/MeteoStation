#include <Arduino.h>
#include <HardwareSerial.h>
#include <queue>
#include <Wire.h>
#include <vector>
#include "ble_setup.h"
#include "AHT20_BMP280_CONTROL.h"
#include <BLEDevice.h> // Temporary for showing the MAC address


//  Heartbeat
unsigned long HeartbeatMillis = 0;
const long Heartbeatinterval = 5000;

// disabled because i use BLE advertising. Can be used if method 3 (ble_setup.cpp) used.
//std::vector<float> temperature_vector;
//std::vector<float> pressure_vector;
//std::vector<float> humidity_vector;


void setup() {
  setCpuFrequencyMhz(80); // lower CPU frequency to cool down cpu


  Serial.begin(115200);
  delay(1000);  // Give time for serial to initialize
  Serial.println("Starting MeteoStation...");
  
  Wire.begin();
  
  // Initialize sensors first
  Serial.println("Initializing sensors...");
  AHT20_begin();
  BMP280_begin();
  startMeasurementAHT20();
  
  // Initialize BLE 
  Serial.println("Initializing BLE..."); 
  initBLE();
  
  Serial.println("Setup complete!");
}

void loop() {
  
  unsigned long currentMillis = millis();
  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;
    
    Serial.println("Device address : " + String(BLEDevice::getAddress().toString().c_str()));

    //BMP280
    readTemperatureBMP280();
    readPressureBMP280();

    Serial.print("BMP280:  Temperature: " + String(temperature_BMP280) + " C");
    Serial.println("  Pressure: " + String(pressure) + " hPa");

    //temperature_vector.push_back(temperature_AHT20);
    //pressure_vector.push_back(pressure);
    
    // AHT20
    startMeasurementAHT20();
    delay(150);
    checkbusyAHT20(); 
    getDataAHT20();

    Serial.print("AHT20:  Humidity: " + String(humidity) + " %");
    Serial.println("Temperature: " + String(temperature_AHT20) + " C ");
    //humidity_vector.push_back(humidity);
    
    // Update BLE characteristics with new sensor data
    //updateBLEData(); 
    broadcastSensorData(); // this one works with app
    //broadcastInDeviceName();

    // Enter light sleep for to cool down the CPU
    Serial.println("Entering light sleep.......................\n\n");
    esp_sleep_enable_timer_wakeup(5000000); // 5 second
    esp_light_sleep_start();
  }
}

