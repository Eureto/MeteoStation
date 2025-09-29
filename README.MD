Esp project created via PlatformIO 

Parts:
- ESP32-C3 super mini
- AHT20+BMP280 sensor (temperature, humidity and pressure)
- 18650 3.7 battery
- 2 in 1 5v battery charger
- switch
- cover

Connection:
Sensors are communicating via i2c bus (check pinout). </br>
Battery charger output gnd to gnd and 5v (through switch) to 5v pin on esp32 

Compile app on android studio and install. </br>
Screenshots of app:
<p align="center">
  <img src="app_dark_theme.jpg" alt="Weather Station Dark Theme" width="250"/>
  <img src="app_light_theme.jpg" alt="Weather Station Light Theme" width="250"/>
</p>
