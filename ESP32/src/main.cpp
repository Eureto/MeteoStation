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

float delta = 0;
float minDelta = 10;
float maxDelta = 0;


std::vector<float> temperature_vector;
std::vector<float> pressure_vector;
std::vector<float> humidity_vector;


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
  // show bluetooth mac address
  Serial.println("Device address : " + String(BLEDevice::getAddress().toString().c_str()));
  Serial.println("Device address : " + String(BLEDevice::getAddress().toString().c_str()));


  checkbusyAHT20();
  getDataAHT20();

  unsigned long currentMillis = millis();
  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;

    //BMP280
    readTemperatureBMP280();
    temperature_vector.push_back(temperature_AHT20);
    Serial.println("BMP280\nTemperature: " + String(temperature_BMP280) + " C");

    readPressureBMP280();
    pressure_vector.push_back(pressure);
    Serial.println("Pressure: " + String(pressure) + " hPa");
 
    // AHT20
    startMeasurementAHT20();
    humidity_vector.push_back(humidity);
    Serial.println("AHT20\nHumidity: " + String(humidity) + " %");
    Serial.println("Temperature: " + String(temperature_AHT20) + " C ");

    // Calculate the delta between the temperature values AHT20 and BMP280
    delta = (temperature_BMP280 > temperature_AHT20) ? (temperature_BMP280 - temperature_AHT20) : (temperature_AHT20 - temperature_BMP280);

    if (delta < minDelta) { minDelta = delta; }
    if (delta > maxDelta) { maxDelta = delta; }
    Serial.print("Temperatur Delta : " + String(delta) + " C");  
    Serial.print(" | Min Delta: " + String(minDelta) + " C");
    Serial.print(" | Max Delta: " + String(maxDelta) + " C\n");
    
    // Update BLE characteristics with new sensor data
    //updateBLEData();
    // broadcastSensorData();
    broadcastInDeviceName();

    Serial.println("Entering light sleep.......................");
    // Enter light sleep for to cool down the CPU
    esp_sleep_enable_timer_wakeup(5000000); // 5 second
    esp_light_sleep_start();
  }
}

