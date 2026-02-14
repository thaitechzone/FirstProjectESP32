# PID Tuning Guide - คู่มือปรับแต่งค่า PID

## ทำความเข้าใจ PID Controller

### PID คืออะไร?

PID (Proportional-Integral-Derivative) เป็นอัลกอริทึมควบคุมที่ใช้กันอย่างแพร่หลายในอุตสาหกรรม โดยมีเป้าหมายเพื่อให้ค่าที่วัดได้ (Process Value - PV) ตรงกับค่าที่ต้องการ (Setpoint - SP)

```
Error = Setpoint - Process Value

Output = Kp×Error + Ki×∫Error·dt + Kd×(dError/dt)
```

### ส่วนประกอบของ PID

#### 1. Proportional (P) - ส่วนที่แปรตามสัดส่วน
```
P_output = Kp × Error
```

**หน้าที่:**
- ตอบสนองตามความผิดพลาดในปัจจุบัน
- ยิ่ง Error มาก P ยิ่งแรง

**ผลกระทบของ Kp:**
- **Kp สูงเกินไป**: 
  - ✗ ระบบแกว่ง (Oscillation)
  - ✗ Overshoot มาก
  - ✓ ตอบสนองเร็ว
  
- **Kp ต่ำเกินไป**:
  - ✓ ระบบเสถียร ไม่แกว่ง
  - ✗ ตอบสนองช้า
  - ✗ มี Steady-state Error

#### 2. Integral (I) - ส่วนที่สะสม
```
I_output = Ki × ∫Error·dt
```

**หน้าที่:**
- กำจัด Steady-state Error (ความผิดพลาดที่เหลืออยู่)
- สะสมค่า Error เมื่อเวลาผ่านไป

**ผลกระทบของ Ki:**
- **Ki สูงเกินไป**: 
  - ✗ Integral Windup (สะสมมากเกินไป)
  - ✗ Overshoot มาก
  - ✗ Recovery ช้า
  
- **Ki ต่ำเกินไป**:
  - ✓ ไม่มีปัญหา Windup
  - ✗ ยังมี Steady-state Error เหลืออยู่

#### 3. Derivative (D) - ส่วนที่คาดการณ์
```
D_output = Kd × (dError/dt)
```

**หน้าที่:**
- คาดการณ์แนวโน้มของ Error
- ลด Overshoot
- เพิ่มความเสถียร

**ผลกระทบของ Kd:**
- **Kd สูงเกินไป**: 
  - ✗ ไวต่อ Noise มาก
  - ✗ ทำให้ระบบสั่น
  
- **Kd ต่ำเกินไป**:
  - ✓ ไม่ไวต่อ Noise
  - ✗ Overshoot มากขึ้น

---

## วิธีการ Tuning

### วิธีที่ 1: Manual Tuning (เหมาะกับผู้เริ่มต้น)

#### Step 1: เริ่มต้นด้วยค่า 0 ทั้งหมด
```cpp
double Kp = 0.0;
double Ki = 0.0;
double Kd = 0.0;
```

#### Step 2: ปรับ Kp จนได้พฤติกรรมที่ต้องการ
1. ตั้ง Setpoint ที่มากกว่าอุณหภูมิปัจจุบัน (เช่น จาก 25°C ไป 50°C)
2. เพิ่ม Kp ทีละนิด (0.5, 1.0, 2.0, 5.0, ...)
3. สังเกตพฤติกรรม:

```
Kp = 0.5  → ตอบสนองช้ามาก
         ▁▂▃▄▅▆▇███───────  (ขึ้นช้า มี Error เยอะ)

Kp = 5.0  → ตอบสนองดี แต่มี Overshoot เล็กน้อย
         ████▇▆▅▆▇███──    (ขึ้นเร็ว แกว่งเล็กน้อย)

Kp = 15.0 → Overshoot มาก แกว่งต่อเนื่อง
         ████████▇▆▅▆▇▆▅▆▇ (แกว่งไม่หยุด)
```

**เป้าหมาย**: หา Kp ที่ตอบสนองเร็ว แต่ยังมี Error เหลืออยู่เล็กน้อย

#### Step 3: เพิ่ม Ki เพื่อกำจัด Steady-state Error
1. เริ่มจาก Ki = Kp/10 (เช่น ถ้า Kp=5.0 ให้ Ki=0.5)
2. เพิ่มทีละนิด และสังเกต:

```
Ki = 0     → มี Error เหลือ 2-3°C
         ████▇▆▅▆▇███──────

Ki = 0.5   → Error ลดลง เหลือ 0.5°C
         ████▇▆▅▆▇████─────

Ki = 2.0   → Error หมด แต่ Overshoot เพิ่มขึ้น
         █████████████▇▆▇█─
```

**เป้าหมาย**: หา Ki ที่ทำให้ Error เหลือน้อยที่สุด โดยไม่ทำให้ Overshoot มากเกินไป

#### Step 4: เพิ่ม Kd เพื่อลด Overshoot
1. เริ่มจาก Kd = Kp/4 (เช่น ถ้า Kp=5.0 ให้ Kd=1.25)
2. ปรับทีละนิด:

```
Kd = 0     → Overshoot 5°C
         ████████▇▆▇████───

Kd = 1.0   → Overshoot 2°C
         ███████▇████──────

Kd = 3.0   → ลด Overshoot มาก แต่ขึ้นช้าลง
         ██████████────────
```

**เป้าหมาย**: หา Kd ที่ลด Overshoot แต่ไม่ทำให้ตอบสนองช้าเกินไป

---

### วิธีที่ 2: Ziegler-Nichols Method (สำหรับผู้ชำนาญ)

#### Ultimate Gain Method

**Step 1:** ตั้งค่าเริ่มต้น
```cpp
double Kp = 0.0;
double Ki = 0.0;
double Kd = 0.0;
```

**Step 2:** หา Ultimate Gain (Ku)
1. เพิ่ม Kp ทีละนิด จนระบบเริ่ม oscillate แบบคงที่
2. บันทึกค่า Kp นี้เป็น Ku

**Step 3:** วัด Oscillation Period (Tu)
1. วัดเวลาของการแกว่ง 1 รอบเต็ม (วินาที)
2. บันทึกค่านี้เป็น Tu

**Step 4:** คำนวณค่า PID
```cpp
// สำหรับ PID Controller
double Kp = 0.6 * Ku;
double Ki = 2 * Kp / Tu;
double Kd = Kp * Tu / 8;
```

#### ตัวอย่างการคำนวณ:
```
สมมติ:
- Ku = 10.0 (ค่า Kp ที่ทำให้แกว่ง)
- Tu = 20 วินาที (เวลา 1 รอบการแกว่ง)

คำนวณ:
Kp = 0.6 × 10.0 = 6.0
Ki = 2 × 6.0 / 20 = 0.6
Kd = 6.0 × 20 / 8 = 15.0
```

---

### วิธีที่ 3: Trial and Error (ทดลองผิดลองถูก)

เหมาะสำหรับระบบที่ไม่ซับซ้อนมาก:

```cpp
// เริ่มต้นด้วยค่าทั่วไปสำหรับระบบความร้อน
double Kp = 5.0;
double Ki = 0.5;
double Kd = 1.0;
```

#### กฎง่ายๆ สำหรับปรับแต่ง:

| ปัญหา | การแก้ไข |
|-------|----------|
| ขึ้นช้าเกินไป | เพิ่ม Kp |
| มี Steady-state Error | เพิ่ม Ki |
| Overshoot มาก | เพิ่ม Kd หรือลด Kp |
| แกว่งไม่หยุด | ลด Kp และ Ki |
| ไวต่อ Noise | ลด Kd |
| ขึ้นเกินแล้วตกกลับไม่หยุด | ลด Ki |

---

## การใช้ Serial Plotter วิเคราะห์

### วิธีดูกราฟ
1. เปิด Serial Monitor หรือ Serial Plotter
2. สังเกตเส้นกราฟ 3 เส้น:
   - **Set** (สีแดง): Setpoint - ค่าที่ต้องการ
   - **PV** (สีเขียว): Process Value - อุณหภูมิจริง
   - **Out** (สีน้ำเงิน): Output - กำลังไฟ Heater (0-255)

### รูปแบบกราฟที่ดี (Good Response)
```
Temp
 │
 │      ┌────────── Setpoint
 │     ╱│
 │    ╱ │
 │   ╱  └─────────── PV (ตามทัน ไม่แกว่งมาก)
 │  ╱
 │ ╱
 └────────────────────── Time
```

### รูปแบบกราฟที่ไม่ดี

#### 1. Kp สูงเกินไป - แกว่งมาก
```
Temp
 │        ┌────────── Setpoint
 │    ╱╲ ╱│╲╱╲╱╲
 │   ╱  ╲╱ │   ╲╱╲   ← PV แกว่งไม่หยุด
 │  ╱              ╲
 │ ╱
 └────────────────────── Time
```

#### 2. Kp ต่ำเกินไป - ขึ้นช้า
```
Temp
 │      ┌────────── Setpoint
 │      │
 │     ╱│           ← ยังไม่ถึง
 │    ╱ │
 │   ╱  │
 │  ╱   │
 └────────────────────── Time
```

#### 3. Ki สูงเกินไป - Overshoot มาก
```
Temp
 │          ╱╲
 │         ╱  ╲
 │      ┌─╱────╲──── Setpoint
 │     ╱ │      ╲
 │    ╱  │       ╲
 │   ╱              ← Overshoot มาก
 └────────────────────── Time
```

---

## ตัวอย่าง PID สำหรับระบบต่างๆ

### 1. ระบบความร้อน - Thermal System (ค่าเริ่มต้นในโค้ด)
```cpp
double Kp = 5.0;   // ค่อนข้างแรง
double Ki = 0.5;   // ค่อยๆ กำจัด Error
double Kd = 1.0;   // ลด Overshoot
```
- เหมาะกับ: ตู้อบ, เครื่องควบคุมอุณหภูมิห้อง
- Thermal Inertia สูง ตอบสนองช้า

### 2. ระบบความร้อนแบบเร็ว - Fast Heating
```cpp
double Kp = 10.0;  // ตอบสนองเร็ว
double Ki = 1.0;   // กำจัด Error เร็ว
double Kd = 2.0;   // ป้องกัน Overshoot
```
- เหมาะกับ: Soldering iron, Hot plate ขนาดเล็ก
- มวลน้อย ตอบสนองเร็ว

### 3. ระบบน้ำอุ่น - Water Heating
```cpp
double Kp = 2.0;   // อ่อนกว่า
double Ki = 0.2;   // ช้าๆ
double Kd = 0.5;   // Damping เบาๆ
```
- เหมาะกับ: เครื่องทำน้ำอุ่น
- มวลมาก ไม่ต้องการการตอบสนองเร็วมาก

---

## แนวทางแก้ปัญหา (Troubleshooting)

### ปัญหา 1: ระบบไม่ขึ้นอุณหภูมิเลย
**สาเหตุ:**
- Output ต่ำเกินไป
- Heater ไม่ทำงาน
- Kp=0

**วิธีแก้:**
```cpp
// ตรวจสอบ Output ใน Serial Monitor
Serial.println(Output);  // ต้องมีค่ามากกว่า 0

// ทดสอบ Heater โดยตรง
ledcWrite(PWM_CHANNEL, 200);  // ทดสอบที่ 80% power
```

### ปัญหา 2: Overshoot มากเกินไป
**วิธีแก้:**
1. ลด Kp ลง 30-50%
2. เพิ่ม Kd ขึ้น 20-30%
3. ถ้ายังไม่หาย ลด Ki ลง

### ปัญหา 3: มี Steady-state Error เหลืออยู่
**วิธีแก้:**
1. เพิ่ม Ki ขึ้นทีละ 0.1
2. ถ้า Overshoot เพิ่มขึ้น ให้เพิ่ม Kd ด้วย

### ปัญหา 4: แกว่งไม่หยุด (Oscillation)
**วิธีแก้:**
1. ลด Kp ลง 50%
2. ลด Ki ลง 50%
3. เพิ่ม Kd ขึ้นเล็กน้อย (10-20%)

---

## เทคนิคขั้นสูง

### 1. Anti-Windup
ป้องกัน Integral Windup เมื่อ Output ถึงขีดจำกัด:

```cpp
// PID Library จาก br3ttb มี Anti-windup built-in
myPID.SetOutputLimits(0, 255);  // จำกัด Output
```

### 2. Setpoint Ramping
ค่อยๆ เปลี่ยน Setpoint เพื่อลด Overshoot:

```cpp
// แทนที่จะกระโดดจาก 25°C → 80°C ทันที
// ค่อยๆ ขึ้นทีละ 1°C ต่อวินาที
if (currentTemp < targetTemp) {
    Setpoint = currentTemp + 1.0;
} else {
    Setpoint = targetTemp;
}
```

### 3. Adaptive PID
ปรับค่า PID อัตโนมัติตามสภาวะ:

```cpp
// เมื่อใกล้ Setpoint ให้ใช้ค่า Kp ต่ำลง
double error = abs(Setpoint - Input);
if (error < 5.0) {
    myPID.SetTunings(Kp*0.5, Ki, Kd);  // ละเอียดขึ้น
} else {
    myPID.SetTunings(Kp, Ki, Kd);      // ปกติ
}
```

---

## บันทึกการ Tuning (Tuning Log)

ใช้แบบฟอร์มนี้บันทึกผลการทดลอง:

```
Date: __________
Application: __________

Test 1:
- Kp: ___  Ki: ___  Kd: ___
- Rise Time: ___ sec
- Overshoot: ___ °C
- Settling Time: ___ sec
- Steady-state Error: ___ °C
- Notes: __________________

Test 2:
- Kp: ___  Ki: ___  Kd: ___
...
```

---

## สรุป Quick Reference

| Parameter | เพิ่มขึ้น | ลดลง |
|-----------|----------|------|
| **Kp** | ตอบสนองเร็วขึ้น, แกว่งมากขึ้น | ตอบสนองช้าลง, แกว่งน้อยลง |
| **Ki** | Error หมดเร็วขึ้น, Overshoot มากขึ้น | Error หมดช้าลง, Overshoot น้อยลง |
| **Kd** | Overshoot ลดลง, ไวต่อ Noise | Overshoot เพิ่มขึ้น, ทนต่อ Noise |

**ค่าแนะนำสำหรับเริ่มต้น (ระบบความร้อน):**
```cpp
double Kp = 5.0;
double Ki = 0.5;
double Kd = 1.0;
```

---

Happy Tuning! 🎯
