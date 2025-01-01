# 🧠 Mega Clever: CO2 & DHT22 Sensor Monitoring System

**Mega Clever** is an Arduino-based project designed to monitor environmental conditions using a **CO2 sensor** and a **DHT22 sensor** (temperature and humidity). The project is ideal for monitoring air quality in a variety of settings.

---

## 📦 Features

- 🌬️ **CO2 Sensor**: Monitors the concentration of carbon dioxide (CO2) in the air.
- 🌡️ **DHT22 Sensor**: Measures the temperature and humidity of the environment.
- 📱 **Real-time Data Display**: Displays sensor readings on **Blynk** app for remote monitoring.

---

## 🛠️ Components Required

- **Arduino Uno** (or compatible board)
- **CO2 Sensor** (e.g., **MG-811**, **MH-Z19**, **CCS811**, etc.)
- **DHT22 Sensor** (Temperature and Humidity sensor)
- Jumper wires
- Breadboard

---

## 🖼️ Circuit Diagram

The wiring setup involves connecting the following components to your Arduino:

### **CO2 Sensor (MG-811 or similar)**

- **VCC** → 5V (Arduino)
- **GND** → GND (Arduino)
- **AOUT** → A3 (Arduino)

### **DHT22 Sensor**

- **VCC** → 5V (Arduino)
- **GND** → GND (Arduino)
- **Data Pin** → Digital Pin 2 (or any other digital pin)

### **ESP8266 (for dev board only)**

- **VCC** → 3.3V (Arduino)
- **GND** → GND (Arduino)
- **TX** → RX (Arduino)
- **RX** → TX (Arduino)

---

## 💻 Installation

### 1. Clone the repository

Clone this repository:

```bash
git clone https://github.com/Nickky464/megaClever.git
```

### 2. Install PlatformIO

If you haven’t already, install **PlatformIO** by following the instructions here: PlatformIO Installation.

You can install PlatformIO as a VS Code extension or as a standalone tool.

### 3. Install Required Libraries

**In PlatformIO:**

- Open the PlatformIO Home and click on Libraries.
- Install the following libraries:
  - Blynk: For integrating the Blynk app.
  - DHT sensor library: For interfacing with the DHT22 sensor.
  - CO2 sensor library (if necessary, depending on your CO2 sensor model).
- Alternatively, you can add them directly in your `platformio.ini` file:

```ini
  lib_deps =
  blynk-library
  DHT sensor library
  <CO2 sensor library>  ; Replace with the library name for your specific CO2 sensor
```

### 4. Configure Blynk App

To enable real-time data monitoring on the Blynk app, follow these steps:

1. Install the Blynk app on your smartphone (available on iOS and Android).
2. Create a new project in the Blynk app and note down the **Template ID**, **Template Name**, and **Auth Token** provided by Blynk.

### 5. Update the Code

In the Arduino code, add the following lines with your specific Blynk credentials:

```cpp
#define BLYNK_TEMPLATE_ID "<YOUR_TEMPLATE_ID>"
#define BLYNK_TEMPLATE_NAME "<YOUR_TEMPLATE_NAME>"                   
#define BLYNK_AUTH_TOKEN "<YOUR_AUTH_TOKEN>"

 ```

 Make sure to replace `<YOUR_TEMPLATE_ID>`, `<YOUR_TEMPLATE_NAME>`, and `<YOUR_AUTH_TOKEN>` with the actual values from the Blynk app.

### 6. Upload the Code to Arduino

Once the code is updated with the correct Blynk details, upload it to your Arduino board using PlatformIO:

1. Open PlatformIO.
2. Click on PlatformIO: Upload in the bottom bar or use the Upload button in the PlatformIO toolbar

### 7. Open the Serial Monitor

After uploading the code, open the Serial Monitor in PlatformIO:

1. Click on **PlatformIO: Serial Monitor** in the bottom bar.
2. Set the baud rate to `9600` (or the baud rate defined in the code).

### 8. Monitor Data on Blynk App

Open the **Blynk** app on your phone, and you should be able to see the real-time data for **CO2 levels**, **temperature**, and **humidity**.

---
This `readme.md` file is not complete.
