#include <Arduino.h>
#include <HardwareSerial.h>
#include <queue>
#include <Wire.h>
#include <vector>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "AHT20_BMP280_CONTROL.h"

// BLE UUIDs for MeteoStation service
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMP_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define HUMIDITY_CHAR_UUID  "beb5483f-36e1-4688-b7f5-ea07361b26a8" 
#define PRESSURE_CHAR_UUID  "beb54840-36e1-4688-b7f5-ea07361b26a8"

// BLE variables
BLEServer* pServer = nullptr;
BLECharacteristic* pTempCharacteristic = nullptr;
BLECharacteristic* pHumidityCharacteristic = nullptr;
BLECharacteristic* pPressureCharacteristic = nullptr;

std::vector<float> temperature_vector;
std::vector<float> pressure_vector;
std::vector<float> humidity_vector;

void initBLE() {
  Serial.println("Starting BLE work!");
  
  BLEDevice::init("MeteoStation");
  pServer = BLEDevice::createServer();
  
  // Create the service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create characteristics for temperature, humidity, and pressure
  pTempCharacteristic = pService->createCharacteristic(
                          TEMP_CHAR_UUID,
                          BLECharacteristic::PROPERTY_READ
                        );
                        
  pHumidityCharacteristic = pService->createCharacteristic(
                              HUMIDITY_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ
                            );
                            
  pPressureCharacteristic = pService->createCharacteristic(
                              PRESSURE_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ
                            );
  
  // Set initial values
  pTempCharacteristic->setValue("0.0°C");
  pHumidityCharacteristic->setValue("0.0%");
  pPressureCharacteristic->setValue("0.0hPa");
  
  // Start the service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE characteristics defined! Now you can read weather data in your phone!");
}

void updateBLEData() {
  if (pTempCharacteristic != nullptr && pHumidityCharacteristic != nullptr && pPressureCharacteristic != nullptr) {
    // Update temperature characteristic
    String tempStr = String(temperature_BMP280, 1) + "°C";
    pTempCharacteristic->setValue(tempStr.c_str());
    
    // Update humidity characteristic  
    String humStr = String(humidity, 1) + "%";
    pHumidityCharacteristic->setValue(humStr.c_str());
    
    // Update pressure characteristic
    String pressStr = String(pressure, 1) + "hPa";
    pPressureCharacteristic->setValue(pressStr.c_str());
    
    Serial.println("BLE data updated: T=" + tempStr + " H=" + humStr + " P=" + pressStr);
  }
}

void list(){
  std::string response = "";
  for (int i = 0; i < temperature_vector.size(); i++) {
    response += "Temp: " + std::to_string(temperature_vector[i]) + "  ";
    response += "Pres: " + std::to_string(pressure_vector[i]) + "  ";
    response += "Hum: " + std::to_string(humidity_vector[i]) + " \n";
  }
}


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
    
    // Update BLE characteristics with new sensor data
    updateBLEData();
  }
}

