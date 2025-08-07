# ATmega128_RearSensor_System

## 📌 Project Overview  
This project is a rear-distance warning system implemented as a final exam assignment in the Microprocessor course.  
It uses an **ultrasonic sensor with an ATmega128 MCU** to detect obstacles behind a vehicle and control **DC motors, LEDs, and a buzzer** accordingly.  
The system mimics a simplified rear sensor unit used in smart vehicles for obstacle detection.

---

## 🧠 Core Features

- Distance measurement using **ultrasonic sensor (TRIG/ECHO)**
- **DC motor** speed control via PWM (dual motors)
- **Auditory alerts** via buzzer based on obstacle distance
- **Visual alerts** via dual LED indicators
- Real-time **distance-based control logic** embedded in firmware

---

## ⚙️ Functional Logic Summary

| Distance Range     | Action                                      |
|--------------------|----------------------------------------------|
| `< 150mm`          | 🔴 Stop motors, blink LED, long buzzer alert |
| `150 ~ 450mm`      | 🟡 Slow motors, LED1 ON, short buzzer alert  |
| `> 450mm`          | 🟢 Fast motors, all LEDs OFF                 |

---

## 🧾 MCU Details

- **MCU**: ATmega128 (16MHz)
- **Language**: Embedded C
- **Development Toolchain**: AVR-GCC
- **Custom Header Used**: `kt_m128.h` (for I/O init)
- **Interrupts**: Not used in this version (polling-based)

---

<img width="487" height="375" alt="image" src="https://github.com/user-attachments/assets/76d2d123-8ef9-4449-b30a-b0651afcbf92" />


---

##🛠️ Hardware Setup

-Ultrasonic Sensor: TRIG on PE6, ECHO on PE7

-DC Motors: Controlled via PD0–PD2 and PD5–PD7

-Buzzer: Controlled via PB4

-LEDs: PC0, PC1 for visual feedback


- **Timers**: Timer1 used for echo pulse width measurement

---
