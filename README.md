## AgriSmart-Device
AgriSmart-Device is a simple IoT solution designed to improve the accuracy of soil nutrient prediction models and help farmers obtain real-time environmental data with ease.

## Repository Contents
This repository contains the ESP32 firmware that:
>Collects sensor data from connected modules.
>Hosts a self-contained web server on the ESP32.
>Updates readings every 2 seconds for live monitoring.

## Hardware Components
1.ESP32
2.Soil Moisture Sensor
3.Temperature & Humidity Sensor (DHT11 or DHT22)

## Setup Instructions
```bash
Install Arduino IDE
Install required libraries
->ESP32 Board Package
->DHT Sensor Library
->Adafruit Unified Sensor Library
```

## How It Works
1. Upload the code to your ESP32 using Arduino IDE.
2. Open the Serial Monitor after upload.
3. Copy the local IP address displayed.
4. Paste the IP address in your web browser to view real-time sensor data.

#Notes
Ensure your ESP32 and your device (phone/laptop) are connected to the same Wi-Fi network.
Data updates automatically every 2 seconds on the hosted web page.
