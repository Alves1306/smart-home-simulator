# Smart Home Simulator 🏠🌡️💡

**Climate Monitoring and Control System using ESP32, MQTT, Node-RED, and ThingSpeak**

This university project simulates an intelligent home automation system with climate control, offering both manual and automatic operation modes. It uses an ESP32 microcontroller, environmental sensors, actuators, a local dashboard via Node-RED, and cloud data visualization through ThingSpeak.

---

## 🔧 Components Used

- ESP32
- DHT22 sensor (indoor temperature and humidity)
- DS18B20 sensor (outdoor temperature)
- PIR sensor (motion detection)
- LDR sensor (light intensity)
- Servo motor for window control
- Servo motor for fan simulation
- Environmental LED
- OLED display (SH1106)

---

## 🧠 Features

- 📡 Publishes sensor data via **MQTT** under topics `home/sensors` and `home/status`
- 🔁 Subscribes to MQTT control commands:
  - `home/fan/set`
  - `home/window/set`
  - `home/light/set`
  - `home/mode` (auto/manual)
- 📊 Real-time local dashboard using **Node-RED**
- ☁️ Sends data to **ThingSpeak** for historical logging and analysis
- 📱 Remote control via MQTT apps or Home Assistant (experimental)

---

## 📁 Project Structure

smart-home-simulator/
├── esp32/ # ESP32 Arduino code
│ └── smart_home.ino
├── node-red/ # Exported Node-RED flows
│ └── flows.json
├── thingspeak/ # API settings or documentation (optional)
├── images/ # Diagrams and screenshots
│ ├── architecture.png
│ ├── dashboard-nodered.png
│ └── dashboard-thingspeak.png
├── docs/ # Final project report
│ └── DTSD_Relatorio.pdf
└── README.md # This file

yaml
Copy
Edit

---

## 🚀 How to Run

1. ⚙️ Flash `esp32/smart_home.ino` to the ESP32 using the Arduino IDE
2. 🔗 Configure your Wi-Fi and MQTT broker IP in the code
3. 🌐 Run Node-RED and import the `flows.json` file
4. 🧪 Connect sensors and actuators to the correct pins:
   - DHT22 → GPIO 4
   - DS18B20 → GPIO 5
   - PIR → GPIO 25
   - LDR → GPIO 34
   - Servo motors → GPIO 18 (window), GPIO 19 (fan)
   - LED → GPIO 14
5. 📲 Open the Node-RED dashboard in your browser
6. ☁️ Optionally configure a ThingSpeak channel to receive data

---

## 🖼️ Screenshots

| Node-RED Dashboard | ThingSpeak Dashboard | OLED Display |
|--------------------|----------------------|--------------|
| ![](images/dashboard-nodered.png) | ![](images/dashboard-thingspeak.png) | ![](images/oled.png) |

---

## ⚠️ Limitations

- No persistent actuator state in case of power loss
- No MQTT authentication (recommended for production use)
- 📱 Remote control via MQTT-compatible apps and **Home Assistant**, integrated with **Google Home** for voice control
---

## 📚 Full Report

For detailed system architecture, UML diagrams, latency measurements, and design decisions, see the full report:  
📄 [`docs/DTSD_Relatorio.pdf`](docs/DTSD_Relatorio.pdf)

---

## 👨‍💻 Authors

- António Alves — 58339  
- João Borges — 70521  
- Tomás Fialho — 60731  
FCT NOVA — Electrical and Computer Engineering  
Academic Year 2024/2025

---

## 📄 License

This project is open-source and available under the MIT License.
