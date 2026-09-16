# 🪑 Smart 4-Seat Booking System (ESP32 IoT + Real-time Web Dashboard)

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![ESP32](https://img.shields.io/badge/Hardware-ESP32-red.svg)
![Node.js](https://img.shields.io/badge/Backend-Node.js%20%7C%20Express-green.svg)
![WebSockets](https://img.shields.io/badge/Realtime-Socket.io-black.svg)
![Frontend](https://img.shields.io/badge/Frontend-TailwindCSS%20%7C%20Chart.js-blueviolet.svg)

An IoT-based **Smart Seat Booking & Telemetry System** that integrates physical seat pressure sensors, a 4x4 keypad matrix, and an interactive real-time web dashboard powered by WebSockets.

---

## 🌟 Key Features

- 🟢 **Live 2x2 Floorplan Map:** Real-time visual seat map updating dynamically when sensors detect weight (Available 🟢 vs Occupied 🔴).
- ⌨️ **On-Site Keypad Booking:** 4x4 matrix keypad connected to ESP32 allowing physical users to request seat reservations.
- 💻 **Web Booking Interface:** Online users can request seat bookings directly from the web dashboard.
- ⚡ **Zero-Latency WebSockets:** Powered by `Socket.io` for bi-directional real-time updates without page refreshing.
- 📜 **Live Activity Stream:** Real-time logging of all keypad entries, sensor telemetries, and web booking requests.
- 📊 **Occupancy Analytics:** Integrated Chart.js graph tracking hourly seat utilization trends.
- ⚙️ **Built-in Hardware Simulator:** Test and demonstrate dashboard functionality directly in the web browser without physical hardware connected.

---

## 🏗️ System Architecture

```
┌─────────────────────────┐       Wi-Fi       ┌──────────────────────────┐
│  ESP32 Microcontroller  │ ────────────────> │  Node.js Backend Server  │
│ 4 Sensors + 4x4 Keypad  │    HTTP POST      │   Express + Socket.io    │
└─────────────────────────┘                   └─────────────┬────────────┘
                                                            │
                                            Socket.io Stream│ (Bi-directional)
                                                            ▼
                                              ┌──────────────────────────┐
                                              │   Real-time Web Dashboard│
                                              │ (Tailwind CSS + Chart.js)│
                                              └──────────────────────────┘
```

---

## 🔌 Hardware Pinout Configuration (ESP32)

| Component | ESP32 Pin | Type | Notes |
| :--- | :--- | :--- | :--- |
| **Seat Sensor 1** | `GPIO 34` | Analog Input | Force Sensing Resistor (FSR) / IR Sensor |
| **Seat Sensor 2** | `GPIO 35` | Analog Input | Force Sensing Resistor (FSR) / IR Sensor |
| **Seat Sensor 3** | `GPIO 32` | Analog Input | Force Sensing Resistor (FSR) / IR Sensor |
| **Seat Sensor 4** | `GPIO 33` | Analog Input | Force Sensing Resistor (FSR) / IR Sensor |
| **Keypad Rows (1-4)** | `GPIO 13, 12, 14, 27` | Digital Matrix | Matrix Rows |
| **Keypad Cols (1-4)** | `GPIO 26, 25, 18, 19` | Digital Matrix | Matrix Columns |

*Note: Threshold is set to `500` (out of 4095 ADC resolution). Reading $> 500$ indicates occupied seat.*

---

## 📁 Repository Structure

```text
smart_seat_booking/
├── esp32_firmware/
│   └── esp32_seat_booking.ino  # C++ Firmware for ESP32
├── server/
│   ├── package.json            # Node.js dependencies
│   └── server.js               # Express + Socket.io backend server
├── public/
│   └── index.html              # Responsive Tailwind CSS Web Dashboard
├── .gitignore                  # Git ignore configuration
└── README.md                   # Project documentation
```

---

## 🚀 Quick Start Guide

### 1. Backend Web Server Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/bavith04/smart-seat-booking-system.git
   cd smart-seat-booking-system/server
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Start the server:
   ```bash
   node server.js
   ```
4. Open your browser at **`http://localhost:3000`**.

### 2. ESP32 Firmware Flashing
1. Open `esp32_firmware/esp32_seat_booking.ino` in Arduino IDE.
2. Edit Wi-Fi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* serverUrlUpdate = "http://<YOUR_PC_IP>:3000/api/seats/update";
   ```
3. Select Board: **ESP32 Dev Module** and click **Upload**.

---

## 📄 License
This project is open-source and available under the [MIT License](LICENSE).
