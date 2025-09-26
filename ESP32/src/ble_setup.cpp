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

// Method 1
// Initializing BLE Advertising
BLEAdvertising* pAdvertising = nullptr;
void initBLE() {
  Serial.println("Starting BLE broadcast mode!");
  
  BLEDevice::init("MeteoStation");
  
  // Set BLE power to minimum for power saving
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);
  
  pAdvertising = BLEDevice::getAdvertising();
  
  Serial.println("BLE advertising initialized!");
  // Serial.println("Device address : " + String(BLEDevice::getAddress().toString().c_str()));
}
void broadcastSensorData() {
  if (pAdvertising != nullptr) {
    // Create custom advertising data with sensor values
    // Format: [temp_int][temp_dec][humidity_int][humidity_dec][pressure_int][pressure_dec]
    
    int temp_int = (int)temperature_BMP280;
    int temp_dec = (int)((temperature_BMP280 - temp_int) * 100);
    int humidity_int = (int)humidity;
    int humidity_dec = (int)((humidity - humidity_int) * 100);
    int pressure_int = (int)pressure;
    int pressure_dec = (int)((pressure - pressure_int) * 100);
    
    std::string sensorData = "";
    sensorData += (char)temp_int;
    sensorData += (char)temp_dec;
    sensorData += (char)humidity_int; 
    sensorData += (char)humidity_dec;
    sensorData += (char)(pressure_int >> 8);  // High byte
    sensorData += (char)(pressure_int & 0xFF); // Low byte
    sensorData += (char)pressure_dec;
    
    pAdvertising->stop();

    BLEAdvertisementData advData;
    advData.setManufacturerData(sensorData);
    pAdvertising->setAdvertisementData(advData);

    pAdvertising->start();
    
    Serial.println("Sensor data broadcasted via BLE advertising");
  }
}

//Method 2
// Broadcasting sensor data in device name
void broadcastInDeviceName() {
  if (pAdvertising != nullptr) {
    // Create device name with sensor data
    String deviceName = "Meteo_T" + String((int)temperature_BMP280) + 
                       "_H" + String((int)humidity) + 
                       "_P" + String((int)pressure);
    
    pAdvertising->stop();
    BLEDevice::deinit();
    BLEDevice::init(deviceName.c_str());
    
    pAdvertising = BLEDevice::getAdvertising();
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);
    pAdvertising->start();
    
    Serial.println("Broadcasting: " + deviceName);
  }
}

// Method 3
// Initialize BLE with MeteoStation service and characteristics
// uses more power
// void initBLE() {
//   Serial.println("Starting BLE work!");
  
//   BLEDevice::init("MeteoStation");

// // Set BLE power to minimum (-12 dBm instead of default +3 dBm)
//   esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);
//   esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N12);
//   esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N12);
  
//   pServer = BLEDevice::createServer();
  
//   // Create the service
//   BLEService *pService = pServer->createService(SERVICE_UUID);
  
//   // Create characteristics for temperature, humidity, and pressure
//   pTempCharacteristic = pService->createCharacteristic(
//                           TEMP_CHAR_UUID,
//                           BLECharacteristic::PROPERTY_READ
//                         );
                        
//   pHumidityCharacteristic = pService->createCharacteristic(
//                               HUMIDITY_CHAR_UUID,
//                               BLECharacteristic::PROPERTY_READ
//                             );
                            
//   pPressureCharacteristic = pService->createCharacteristic(
//                               PRESSURE_CHAR_UUID,
//                               BLECharacteristic::PROPERTY_READ
//                             );
  
//   // Set initial values
//   pTempCharacteristic->setValue("0.0°C");
//   pHumidityCharacteristic->setValue("0.0%");
//   pPressureCharacteristic->setValue("0.0hPa");
  
//   // Start the service
//   pService->start();
  
//   // Start advertising
//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->setScanResponse(true);
//   pAdvertising->setMinPreferred(0x320); // 0x320 = 800, 1s
//   pAdvertising->setMaxPreferred(0x640); // 0x640 = 1600, 2s
  
//   BLEDevice::startAdvertising();
  
//   Serial.println("BLE characteristics defined! Now you can read weather data in your phone!");
// }

// void updateBLEData() {
//   if (pTempCharacteristic != nullptr && pHumidityCharacteristic != nullptr && pPressureCharacteristic != nullptr) {
//     // Update temperature characteristic
//     String tempStr = String(temperature_BMP280, 1) + "°C";
//     pTempCharacteristic->setValue(tempStr.c_str());
    
//     // Update humidity characteristic  
//     String humStr = String(humidity, 1) + "%";
//     pHumidityCharacteristic->setValue(humStr.c_str());
    
//     // Update pressure characteristic
//     String pressStr = String(pressure, 1) + "hPa";
//     pPressureCharacteristic->setValue(pressStr.c_str());
    
//     Serial.println("BLE data updated: T=" + tempStr + " H=" + humStr + " P=" + pressStr);
//   }
// }

