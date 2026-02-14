# Quick Start Guide - เริ่มต้นใช้งานด่วน

ใช้งานระบบ PID Controller ได้ใน 5 นาที!

---

## ขั้นตอนที่ 1: ตรวจสอบอุปกรณ์ ✓

คุณต้องมี:
- [ ] ESP32 DevKit V2
- [ ] DS18B20 Temperature Sensor
- [ ] OLED Display 0.96" (I2C)
- [ ] MOSFET (IRLZ44N) หรือ SSR
- [ ] Heater Element
- [ ] สาย Jumper
- [ ] Resistor 4.7kΩ (สำหรับ DS18B20)
- [ ] Resistor 220Ω (สำหรับ MOSFET)

---

## ขั้นตอนที่ 2: ต่อสาย 🔌

### การต่อขาแบบง่าย:

```
DS18B20:
  • VCC → ESP32 3.3V
  • Data → ESP32 GPIO 14 (+ Pull-up 4.7kΩ ไป 3.3V)
  • GND → ESP32 GND

OLED Display:
  • VCC → ESP32 3.3V
  • GND → ESP32 GND
  • SCL → ESP32 GPIO 22
  • SDA → ESP32 GPIO 21

MOSFET (IRLZ44N):
  • Gate → 220Ω → ESP32 GPIO 13
  • Source → GND (Common Ground)
  • Drain → Heater → Power Supply

⚠️ สำคัญ: ต้องใช้ Common Ground ทุกส่วน!
```

---

## ขั้นตอนที่ 3: ติดตั้งซอฟต์แวร์ 💻

### สำหรับ Windows/Mac/Linux:

```bash
# 1. ติดตั้ง PlatformIO
pip install platformio

# 2. Clone โปรเจค
git clone https://github.com/thaitechzone/FirstProjectESP32.git
cd FirstProjectESP32

# 3. Build
platformio run

# 4. Upload ไป ESP32
platformio run --target upload

# 5. เปิด Serial Monitor
platformio device monitor
```

---

## ขั้นตอนที่ 4: ทดสอบครั้งแรก 🧪

### 4.1 ตรวจสอบ Sensor
1. เปิด Serial Monitor (115200 baud)
2. คุณควรเห็น:
   ```
   --- ESP32 PID Ready ---
   Set:40.0,PV:25.3,Out:255
   ```
3. ตรวจสอบว่า `PV` (Process Value) แสดงอุณหภูมิปัจจุบันถูกต้อง

### 4.2 ตรวจสอบ OLED
- จอควรแสดง:
  - อุณหภูมิปัจจุบัน (ตัวใหญ่)
  - Setpoint (ค่าที่ตั้ง)
  - Power % (กำลังไฟ)
  - ค่า PID (P, I, D)

### 4.3 ทดสอบ Heater (ไม่ต้องต่อ Heater จริงก่อน)
1. ต่อ LED ที่ GPIO 13 (ผ่าน Resistor 220Ω)
2. LED ควรสว่างตามกำลังไฟที่ Output
3. ลอง เปลี่ยน Setpoint ผ่าน Serial:
   ```
   60<Enter>
   ```
4. LED ควรสว่างขึ้น (เพราะ Output เพิ่มขึ้น)

---

## ขั้นตอนที่ 5: เริ่มใช้งานจริง 🚀

### 5.1 ต่อ Heater จริง
⚠️ **คำเตือน**: ตรวจสอบแรงดันและกระแสของ Heater ให้ตรงกับ MOSFET/SSR

### 5.2 ตั้งค่าอุณหภูมิ
ผ่าน Serial Monitor:
```
50<Enter>     → ตั้งที่ 50°C
75.5<Enter>   → ตั้งที่ 75.5°C
```

### 5.3 สังเกตการทำงาน
- **Rise Time**: เวลาที่ใช้จากอุณหภูมิเริ่มต้นถึง Setpoint
- **Overshoot**: อุณหภูมิเกินกว่า Setpoint ไปเท่าไหร่
- **Steady-state**: อุณหภูมิคงที่หลังจากถึง Setpoint

---

## ปัญหาที่พบบ่อยและวิธีแก้ 🔧

### ปัญหา 1: OLED ไม่แสดงผล
**แก้ไข:**
```cpp
// ใน src/main.cpp บรรทัด 87
// ลองเปลี่ยน I2C address จาก 0x3C เป็น 0x3D
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
```

### ปัญหา 2: Sensor อ่านค่า -127°C
**สาเหตุ:** 
- ไม่มี Pull-up Resistor 4.7kΩ
- สายต่อหลุด
- Sensor เสีย

**แก้ไข:**
- ตรวจสอบ Pull-up Resistor
- ลองใช้สายที่สั้นกว่า
- ทดสอบ Sensor ด้วย Example Code ของ DallasTemperature

### ปัญหา 3: Heater ไม่ทำงาน
**แก้ไข:**
```cpp
// ทดสอบ PWM โดยตรง
void loop() {
  ledcWrite(PWM_CHANNEL, 200);  // ฝืนให้ Output = 200
  delay(1000);
}
```
- ถ้า Heater ทำงาน → ปัญหาอยู่ที่ PID
- ถ้าไม่ทำงาน → ตรวจสอบการต่อสาย MOSFET/SSR

### ปัญหา 4: อุณหภูมิแกว่งไม่หยุด
**แก้ไข:**
```cpp
// ลดค่า Kp และ Ki ลง 50%
double Kp = 2.5;  // จาก 5.0
double Ki = 0.25; // จาก 0.5
double Kd = 1.0;
```

---

## การปรับแต่งขั้นสูง ⚙️

### เปลี่ยนค่า PID เริ่มต้น
```cpp
// ในไฟล์ src/main.cpp บรรทัด 64
double Kp = 5.0;   // เปลี่ยนตามต้องการ
double Ki = 0.5;
double Kd = 1.0;
```

### เปลี่ยน Setpoint เริ่มต้น
```cpp
// ในไฟล์ src/main.cpp บรรทัด 104
Setpoint = 40.0;   // เปลี่ยนจาก 40 เป็นค่าที่ต้องการ
```

### เปลี่ยนขอบเขตอุณหภูมิ
```cpp
// ในไฟล์ src/main.cpp บรรทัด 41-42
#define TEMP_MIN 25.0     // ต่ำสุด
#define TEMP_MAX 100.0    // สูงสุด (Safety Cutoff)
```

### เปลี่ยนความถี่ PWM
```cpp
// ในไฟล์ src/main.cpp บรรทัด 45
const int PWM_FREQ = 1000;  // 1kHz (สำหรับ MOSFET)
// สำหรับ SSR ควรใช้ 10-50 Hz
```

---

## เคล็ดลับการใช้งาน 💡

### 1. Serial Plotter
ใช้ดูกราฟแบบ Real-time:
```
Tools → Serial Plotter (Arduino IDE)
```
คุณจะเห็นเส้นกราฟ 3 เส้น: Set, PV, Out

### 2. บันทึกข้อมูล
Copy ข้อมูลจาก Serial Monitor ไปวิเคราะห์ใน Excel:
```
Set:50.0,PV:48.5,Out:180
Set:50.0,PV:48.9,Out:165
Set:50.0,PV:49.2,Out:148
```

### 3. PID Tuning
อ่านคู่มือ PID_TUNING.md สำหรับวิธีการละเอียด

### 4. ตัวอย่างการใช้งาน
ดู APPLICATIONS.md สำหรับ 8 ตัวอย่างการประยุกต์ใช้

---

## Next Steps 📚

เรียนรู้เพิ่มเติม:
1. [README.md](README.md) - คู่มือฉบับสมบูรณ์
2. [WIRING.md](WIRING.md) - วงจรและการต่อสายละเอียด
3. [PID_TUNING.md](PID_TUNING.md) - คู่มือปรับแต่ง PID
4. [APPLICATIONS.md](APPLICATIONS.md) - ตัวอย่างการใช้งานจริง

---

## ขอความช่วยเหลือ 🆘

ถ้ามีปัญหา:
1. ตรวจสอบ [README.md](README.md) ส่วน Troubleshooting
2. เปิด Issue ใน GitHub
3. ติดต่อ THAITECHZONE Community

---

**สนุกกับการทำโปรเจค! Happy Making! 🎉**

---

## Checklist ก่อนเริ่มใช้งาน

- [ ] ต่อ DS18B20 ถูกต้อง (มี Pull-up 4.7kΩ)
- [ ] ต่อ OLED ถูกต้อง (I2C: SDA=21, SCL=22)
- [ ] ต่อ MOSFET ถูกต้อง (Gate → GPIO13 ผ่าน 220Ω)
- [ ] ใช้ Common Ground ทุกส่วน
- [ ] Upload โค้ดเรียบร้อย
- [ ] Sensor อ่านค่าถูกต้อง (ไม่ใช่ -127 หรือ 85)
- [ ] OLED แสดงผลได้
- [ ] ทดสอบ PWM Output ด้วย LED
- [ ] ตรวจสอบความปลอดภัยของ Heater
- [ ] ติดตั้ง Thermal Fuse (แนะนำ)
- [ ] พร้อมใช้งาน! 🚀
