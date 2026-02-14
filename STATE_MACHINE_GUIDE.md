# State Machine Guide - ESP32 PID Temperature Controller

## Overview
โปรแกรมนี้ใช้ State Machine เพื่อจัดการโหมดการทำงานต่างๆ ของระบบควบคุมอุณหภูมิ PID

## Hardware Setup
- **SW1 (GPIO 34)** - ปุ่ม Mode/Enter (ต้องต่อ R Pull-up 10kΩ ภายนอก)
- **SW2 (GPIO 35)** - ปุ่ม Down (ต้องต่อ R Pull-up 10kΩ ภายนอก)  
- **SW3 (GPIO 32)** - ปุ่ม Up (มี Internal Pull-up)

## State Machine Flow

```
┌─────────────────┐
│                 │
│   Processing    │ ◄──┐
│   (เริ่มต้น)    │    │
│                 │    │
└────────┬────────┘    │
         │ SW1         │
         ▼             │
┌─────────────────┐    │
│  SetPoint       │    │
│  Config         │    │
│                 │    │
└────────┬────────┘    │
         │ SW1         │
         ▼             │
┌─────────────────┐    │
│  PID Config     │    │
│  (P, I, D)      │    │
│                 │    │
└────────┬────────┘    │
         │ SW1         │
         ▼             │
┌─────────────────┐    │
│  Manual PWM     │    │
│  Test           │    │
│                 │    │
└────────┬────────┘    │
         │ SW1         │
         └─────────────┘
```

## States Description

### 1. State_Processing (โหมดควบคุมอัตโนมัติ)
**หน้าที่:** ควบคุมอุณหภูมิด้วย PID อัตโนมัติ

**การแสดงผล:**
- อุณหภูมิปัจจุบัน (PV)
- ค่า Setpoint
- กำลังงาน Heater (%)
- ค่า P, I, D
- กราฟแท่งแสดงกำลังงาน

**การใช้งาน:**
- กด **SW1** → ไปโหมดตั้งค่า Setpoint
- สามารถส่งค่า Setpoint ผ่าน Serial ได้ (เช่น พิมพ์ 60.5)

---

### 2. State_SetPoint_Config (โหมดตั้งค่า Setpoint)
**หน้าที่:** ปรับค่า Setpoint ด้วยปุ่ม

**การแสดงผล:**
```
CONFIG: SETPOINT
SP: 40.0
PV: 25.5
UP/DN: +/-  SW1:Next
```

**การใช้งาน:**
- กด **SW3 (Up)** → เพิ่มค่า Setpoint +1°C
- กด **SW2 (Down)** → ลดค่า Setpoint -1°C
- กด **SW1** → ไปโหมด PID Config
- ขอบเขต: 25-100°C

---

### 3. State_PID_Config (โหมดตั้งค่า PID)
**หน้าที่:** ปรับค่า P, I, D parameters

**การแสดงผล:**
```
CONFIG: PID TUNING
> Kp: 5.00
  Ki: 0.50
  Kd: 1.00
SW1:Next (1/4)
```

**การใช้งาน:**
1. กด **SW3 (Up)** → เพิ่มค่า parameter ที่แสดง (มี >) +0.1
2. กด **SW2 (Down)** → ลดค่า parameter ที่แสดง (มี >) -0.1
3. กด **SW1** → เลือก parameter ถัดไป (Kp → Ki → Kd → Kp)
4. กด **SW1** ครบ 4 ครั้ง → ไปโหมด Manual PWM

**หมายเหตุ:**
- เครื่องหมาย `>` แสดง parameter ที่กำลังปรับค่าอยู่
- ตัวเลข (X/4) แสดงจำนวนครั้งที่กด SW1 แล้ว
- ค่าต่ำสุดของแต่ละ parameter คือ 0
- หลังกด SW1 ครบ 4 ครั้ง จะไปโหมด Manual PWM อัตโนมัติ

---

### 4. State_Manual_PWM (โหมดทดสอบ PWM)
**หน้าที่:** ควบคุม PWM แบบ Manual (ไม่ใช้ PID)

**การแสดงผล:**
```
MANUAL PWM TEST
PWM:128
Power: 50%
Temp: 25.5C
UP/DN:+/-  SW1:Exit
```

**การใช้งาน:**
- กด **SW3 (Up)** → เพิ่มค่า PWM +10 (0-255)
- กด **SW2 (Down)** → ลดค่า PWM -10
- กด **SW1** → กลับโหมด Processing

**คำเตือน:**
- ใช้โหมดนี้ด้วยความระมัดระวัง
- ตรวจสอบอุณหภูมิอยู่เสมอเพื่อป้องกันความเสียหาย

---

### 5. State_Display (สำรองไว้ใช้ในอนาคต)
State นี้ยังไม่ได้ใช้งานในโหมดหลัก แต่สามารถเรียกใช้ฟังก์ชัน `stateDisplay()` ได้

---

## Button Debouncing
- ระบบมี debounce delay 200ms เพื่อป้องกันการกดปุ่มซ้ำ
- ถ้าปุ่มไม่ตอบสนอง รอสักครู่แล้วกดใหม่

## Serial Monitor Commands
ในโหมด Processing สามารถส่งคำสั่งผ่าน Serial Monitor:
```
40.5    → ตั้ง Setpoint = 40.5°C
60      → ตั้ง Setpoint = 60°C
```

## Safety Features
ระบบมีการป้องกันความปลอดภัยในทุกโหมด:
- ตัด Heater ทันทีเมื่อ Sensor เสีย
- ตัด Heater ทันทีเมื่ออุณหภูมิเกิน 100°C
- จำกัด Setpoint ระหว่าง 25-100°C

## Tips for PID Tuning
1. เริ่มต้นด้วย Kp = 5, Ki = 0.5, Kd = 1
2. ถ้า overshoot มาก → ลด Kp
3. ถ้า settle ช้า → เพิ่ม Ki
4. ถ้า oscillate → ปรับ Kd

## Troubleshooting
- **ปุ่มไม่ทำงาน:** ตรวจสอบ Pull-up resistor 10kΩ ที่ SW1, SW2
- **จอไม่แสดงผล:** ตรวจสอบการเชื่อมต่อ I2C (SDA=21, SCL=22)
- **อุณหภูมิ -127°C:** Sensor DS18B20 ไม่ได้ต่อหรือเสีย
