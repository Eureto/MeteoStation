#include "ble_setup.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>
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