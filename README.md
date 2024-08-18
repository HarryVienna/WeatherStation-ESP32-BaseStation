# WeatherStation-ESP32-BaseStation 2.0

New implementation for MaTouch IPS (1024x600) panel. I migrate the whole code from Arduino to ESP-IDF to be able to use the newest display drivers for parallel displays

- Base station with ESP32-S3 and 7-inch IPS display.
- Setup screen for configuring the weather station.
- Wireless sensors based on ESP32. There are three possible sensor variants.
- Battery of the wireless sensors is charged via solar cells.
- Very low power consumption of the wireless sensors thanks to deep-sleep mode and ESP-NOW protocol.
- Depending on the wireless sensor, 4 or 8 sensors can be configured using a DIP switch.
- Wireless sensors connect to the base station fully automatically.
- Each wireless sensor measures temperature, humidity and air pressure.
- The base station provides temperature, humidity, VOC index, particulate matter, NOx and CO2 content.
- Detailed weather forecast via the Open-Meteo API for the next 48 hours or 7 days.
- The display automatically adjusts to the brightness of the surroundings.
- If nobody is near the weather station, the display is automatically dimmed.
- Housing from the 3D printer, so that the weather station is also suitable for the living room.


You can find a detailed description of the project [on my website](https://www.haraldkreuzer.net/en/news/esp32-weather-station-20-radio-sensors-open-meteo-api-ips-display-mmwave-radar-fine-dust-sensor-and-much-more) 

![DSCF4802](https://github.com/user-attachments/assets/74ae9b6d-ce46-412a-83e6-d0010ac6d0b0)





