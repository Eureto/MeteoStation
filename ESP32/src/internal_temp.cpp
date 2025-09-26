#include <Arduino.h>
#include "internal_temp.h"

#ifdef __cplusplus
extern "C" {
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

static float internal_temperature = 0.0;

void readInternalTemperature() {
    // Read raw temperature value
    uint8_t raw_temp = temprature_sens_read();
    
    // Convert to Celsius (approximate formula for ESP32-C3)
    // This is a rough calibration - actual formula may vary
    internal_temperature = (raw_temp - 32) / 1.8;
    
    Serial.print("Internal Chip Temperature: ");
    Serial.print(internal_temperature, 1);
    Serial.println(" °C (approximate)");
}

float getInternalTemperature() {
    uint8_t raw_temp = temprature_sens_read();
    return (raw_temp - 32) / 1.8;
}

