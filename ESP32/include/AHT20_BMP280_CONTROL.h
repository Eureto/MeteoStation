#ifndef AHT20_BMP280_CONTROL_H
#define AHT20_BMP280_CONTROL_H

// Include necessary standard libraries
#include <stdint.h>
#include <stdbool.h>

// Include Arduino/ESP32 specific headers if needed
// #include <Arduino.h>

// BMP280 Sensor Variables
/*******************************/
extern float temperature_BMP280;
extern float pressure;


// BMP280 Temperature compensation variable
extern int32_t _t_fine;

// BMP280 Calibration/Trimming parameters
extern uint16_t _dig_T1;
extern int16_t _dig_T2;
extern int16_t _dig_T3;

extern uint16_t _dig_P1;
extern int16_t _dig_P2;
extern int16_t _dig_P3;
extern int16_t _dig_P4;
extern int16_t _dig_P5;
extern int16_t _dig_P6;
extern int16_t _dig_P7;
extern int16_t _dig_P8;
extern int16_t _dig_P9;
/*******************************/

void scanI2CDevices();

//AHT20 Sensor Variables
/*******************************/
extern float temperature_AHT20;
extern float humidity;
extern bool sensor_started;
extern bool sensor_busy;
extern unsigned long measurementDelayAHT20;
/*******************************/


// Function Declarations
/*******************************/

// AHT20 Sensor Functions
void AHT20_begin();
void startMeasurementAHT20();
void checkbusyAHT20();
void getDataAHT20();

// BMP280 Sensor Functions
void BMP280_begin();
void readTemperatureBMP280();
void readPressureBMP280();

// Utility Functions
void readAndDisplayRegister(uint8_t deviceAddress, uint8_t registerAddress, const char* registerName);

/*******************************/

#endif // AHT20_BMP280_CONTROL_H
