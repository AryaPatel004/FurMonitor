# FurMonitor

A wearable biometric monitoring system for dogs that measures **heart rate** and **blood oxygen (SpO2)** in real time using an ESP32 and the SparkFun MAX30101/MAX32664 pulse oximeter board. Features a custom algorithm developed to work around the hardware limitations of the built-in MAX32664 biosensor hub.

---

## 🐾 Overview

The SparkFun Bio Sensor Hub (MAX32664) comes with a built-in algorithm locked to fingertip placement, making it unreliable for pets. This project bypasses that limitation by reading raw IR and Red LED values directly from the MAX30101 sensor and computing heart rate and SpO2 using a custom algorithm designed for motion-prone environments.

---

## ✨ Key Features

- **Custom SpO2 algorithm** using the Ratio of Ratios method with AC/DC signal separation via a moving average filter
- **Custom heart rate algorithm** using IR peak detection with smoothing and a circular averaging buffer
- **Dynamic reset** — automatically clears readings when the sensor is removed
- Both library-based and custom algorithm outputs printed side-by-side for comparison
- Configurable pulse width and sample rate

---

## ⚙️ How the Custom Algorithms Work

### SpO2 (Blood Oxygen)
The raw Red and IR LED signals each contain a DC component (baseline tissue absorption) and an AC component (pulsatile blood flow). The algorithms separates these using a low-pass moving average filter, then computes the Ratio of Ratios (R) and applies an empirical calibration curve:

```
R = (AC_red / DC_red) / (AC_ir / DC_ir)
SpO2 = (-45.06 × R²) + (30.35 × R) + 94.85
```

### Heart Rate
A smoothed IR signal is monitored for threshold crossings. When a peak is detected (IR > 50,000 counts, with a minimum 600ms gap between beats), the time delta is used to calculate BPM. A circular buffer of 4 readings is maintained and averaged for stability. If the sensor is removed (both IR and Red drop below 10,000), all buffers are cleared automatically.

---

## 🔧 Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 (any variant) |
| Sensor Board | SparkFun Pulse Oximeter & Heart Rate — MAX30101 + MAX32664 |
| Interface | I²C |
| Reset Pin | GPIO 4 |
| MFIO Pin | GPIO 5 |

### Sensor Configuration Used
| Parameter | Value |
|---|---|
| Pulse Width | 411 µs |
| Sample Rate | 400 samples/sec |
| Mode | MODE_ONE (BPM mode) |

---

## 📁 Repository Structure

```
furmonitor/
│
├── firmware/
│   └── furmonitor.ino           # Main Arduino sketch (ESP32)
│
├── docs/
│   └── sensor_limitation.png   # Forum screenshot confirming green LED restriction
│
├── .gitignore
└── README.md
```

---

## 🚀 Setup & Installation

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support
- [SparkFun Bio Sensor Hub Library](https://github.com/sparkfun/SparkFun_Bio_Sensor_Hub_Library) — install via Arduino Library Manager

### Steps
1. Install the SparkFun Bio Sensor Hub Library:
   - Arduino IDE → **Sketch → Include Library → Manage Libraries**
   - Search for `SparkFun Bio Sensor Hub` and install
2. Open `firmware/furmonitor.ino` in Arduino IDE
3. Select your ESP32 board under **Tools → Board**
4. Connect the MAX30101/MAX32664 board to ESP32 via I²C:
   - SDA → GPIO 21
   - SCL → GPIO 22
   - Reset → GPIO 4
   - MFIO → GPIO 5
5. Upload and open the Serial Monitor at **115200 baud**

---

## 📊 Serial Monitor Output

```
Infrared LED counts: 87432
Red LED counts: 72100
Heartrate using library: 74
Heartrate using algorithm: 76.25
Blood Oxygen using library: 97
Blood oxygen using algorithm: 96.84
Confidence: 100
Status: 3
```

---

## ⚠️ Known Limitations

- The MAX32664 does not support the green LED — it is locked to IR and Red only under Firmware Version A. This is a hardware restriction confirmed by SparkFun.
- The built-in library algorithm is optimized for fingertip placement and does not work reliably on dogs. The custom algorithm addresses this but is still a work in progress for heavy motion scenarios.
- The SpO2 empirical equation is calibrated for general use and may need tuning for specific animals or fur densities.

---

## 🛠️ Built With

- [Arduino IDE](https://www.arduino.cc/) — Firmware development
- [SparkFun Bio Sensor Hub Library](https://github.com/sparkfun/SparkFun_Bio_Sensor_Hub_Library) — Sensor interface
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) — ESP32 support

---

## 👤 Author

**Arya Sureshbhai Patel**  
