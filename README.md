# ESP32 PID Temperature Controller

โครงการควบคุมอุณหภูมิด้วย PID Controller บน ESP32 (Professional Edition)

## ภาพรวมโครงการ (Project Overview)

ระบบควบคุมอุณหภูมิแบบอัตโนมัติที่ใช้ PID Control Algorithm เพื่อควบคุมอุณหภูมิให้คงที่ตามค่าที่ตั้งไว้ (Setpoint) เหมาะสำหรับงานที่ต้องการความแม่นยำสูง เช่น:
- เครื่องควบคุมอุณหภูมิห้อง
- ระบบควบคุมอุณหภูมิในกระบวนการผลิต
- เครื่องควบคุมอุณหภูมิสำหรับการทดลองทางวิทยาศาสตร์
- ระบบอบแห้ง (Drying System)

## Hardware Requirements

### ส่วนประกอบหลัก (Main Components)
1. **ESP32 DevKit V2** - ไมโครคอนโทรลเลอร์หลัก
2. **DS18B20** - เซ็นเซอร์วัดอุณหภูมิแบบดิจิทัล (ความแม่นยำ ±0.5°C)
3. **OLED Display 0.96"** - จอแสดงผล I2C (128x64 pixels)
4. **MOSFET หรือ SSR** - สวิตช์กำลังสำหรับควบคุม Heater
5. **Heater Element** - องค์ประกอบความร้อน (เช่น หลอดความร้อน, ฮีตเตอร์)
6. **แหล่งจ่ายไฟ** - สำหรับ ESP32 (5V/1A) และ Heater (ตามความเหมาะสม)

### การต่อขา (Pin Connections)

```
ESP32          →  อุปกรณ์
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO 13        →  MOSFET/SSR Gate (PWM Output)
GPIO 14        →  DS18B20 Data (ต้องต่อ Pull-up 4.7kΩ ไป VCC)
GPIO 21        →  OLED SDA
GPIO 22        →  OLED SCL
GND            →  Ground ทุกตัว (Common Ground)
3.3V           →  DS18B20 VCC, OLED VCC

ขาสำรอง (Optional):
GPIO 34, 35, 32 → Switch Inputs (สำหรับควบคุมเพิ่มเติม)
GPIO 17, 16, 4  → Relay Outputs (สำหรับฟังก์ชันเพิ่มเติม)
```

### DS18B20 Wiring
```
DS18B20 ดูจากด้านหน้า (Flat side):
    ___________
   |  __ __ __ |
   | |  |  |  ||
   | 1  2  3   |
   |___________|

Pin 1: GND
Pin 2: Data (ไป GPIO 14 + Pull-up 4.7kΩ)
Pin 3: VCC (3.3V)
```

## Software Requirements

### PlatformIO Configuration
โปรเจคนี้ใช้ PlatformIO และมีการติดตั้ง Library อัตโนมัติ:

```ini
lib_deps =
    paulstoffregen/OneWire @ ^2.3.8           # สำหรับสื่อสาร DS18B20
    milesburton/DallasTemperature @ ^4.0.6     # Driver DS18B20
    adafruit/Adafruit SSD1306 @ ^2.5.9         # Driver OLED
    adafruit/Adafruit GFX Library @ ^1.11.9    # กราฟิกส์ไลบรารี
    br3ttb/PID @ ^1.2.1                        # PID Controller
```

## Features (คุณสมบัติ)

### 1. ระบบควบคุม PID
- **Proportional (P)**: ตอบสนองตามความแตกต่างระหว่างค่าจริงกับค่าที่ตั้ง
- **Integral (I)**: กำจัด Steady-state Error
- **Derivative (D)**: ลดการ Overshoot

### 2. ระบบความปลอดภัย (Safety Features)
- **Overheat Protection**: ตัดไฟอัตโนมัติเมื่ออุณหภูมิเกิน 100°C
- **Sensor Error Detection**: ตรวจจับเซ็นเซอร์เสีย และหยุดทำงานทันที
- **PWM Output Limits**: จำกัดค่าเอาต์พุต 0-255 (0-100%)

### 3. User Interface
- **OLED Display**:
  - อุณหภูมิปัจจุบัน (PV - Process Value) แบบตัวใหญ่
  - อุณหภูมิเป้าหมาย (Setpoint)
  - กำลังไฟฮีตเตอร์ (% Power)
  - ค่าพารามิเตอร์ PID (Kp, Ki, Kd)
  - กราฟแท่งแสดงกำลังไฟ Real-time

- **Serial Monitor**:
  - เปลี่ยนค่า Setpoint ได้ทาง Serial (พิมพ์ตัวเลข เช่น 60.5)
  - แสดงข้อมูลสำหรับ Serial Plotter

### 4. การควบคุมผ่าน Serial
```
1. เปิด Serial Monitor (Baud rate: 115200)
2. พิมพ์อุณหภูมิที่ต้องการ เช่น 60.5 (25-100°C)
3. กด Enter
```

## การติดตั้งและใช้งาน (Installation & Usage)

### ขั้นตอนที่ 1: เตรียม Environment
```bash
# Install PlatformIO Core
pip install platformio

# หรือติดตั้ง PlatformIO IDE
# https://platformio.org/platformio-ide
```

### ขั้นตอนที่ 2: Clone Project
```bash
git clone https://github.com/thaitechzone/FirstProjectESP32.git
cd FirstProjectESP32
```

### ขั้นตอนที่ 3: Build & Upload
```bash
# Build โปรเจค
platformio run

# Upload ไปยัง ESP32
platformio run --target upload

# เปิด Serial Monitor
platformio device monitor
```

### ขั้นตอนที่ 4: การทดสอบ
1. เปิด Serial Monitor (115200 baud)
2. ตรวจสอบว่าเซ็นเซอร์อ่านค่าได้ปกติ
3. ทดสอบเปลี่ยนค่า Setpoint ผ่าน Serial
4. สังเกตการทำงานของ Heater และการตอบสนองของระบบ

## PID Tuning (การปรับแต่ง PID)

### ค่าเริ่มต้น (Default Values)
```cpp
double Kp = 5.0;   // Proportional gain
double Ki = 0.5;   // Integral gain  
double Kd = 1.0;   // Derivative gain
```

### วิธีการ Tuning (Ziegler-Nichols Method)

1. **ตั้งค่าเริ่มต้น**: Ki=0, Kd=0, เพิ่ม Kp จนระบบเริ่ม oscillate
2. **หา Ultimate Gain (Ku)**: ค่า Kp ที่ทำให้เกิด oscillation คงที่
3. **หา Period (Tu)**: วัดเวลา 1 รอบของการแกว่ง
4. **คำนวณค่า PID**:
   - Kp = 0.6 × Ku
   - Ki = 2 × Kp / Tu
   - Kd = Kp × Tu / 8

### เทคนิคการปรับแต่งแบบ Manual
```
- ระบบตอบสนองช้า → เพิ่ม Kp
- มี Steady-state Error → เพิ่ม Ki
- มี Overshoot มาก → เพิ่ม Kd หรือลด Kp
- Oscillate → ลด Kp และ Ki
```

## การใช้งาน Serial Plotter

ระบบส่งข้อมูลในรูปแบบที่เหมาะกับ Arduino Serial Plotter:
```
Set:50.0,PV:48.5,Out:180
```

### วิธีดู Serial Plotter:
1. Arduino IDE: Tools → Serial Plotter
2. PlatformIO: ใช้ `platformio device monitor` แล้วคัดลอกไปวิเคราะห์

## ตัวอย่างการทำงาน (Example Output)

### Serial Monitor Output
```
--- ESP32 PID Ready ---
Type temp in Serial (e.g., 60.5) to change Setpoint
Set:40.0,PV:25.3,Out:255
Set:40.0,PV:27.8,Out:255
Set:40.0,PV:30.2,Out:240
Set:40.0,PV:33.1,Out:220
...
New Setpoint: 50.0
Set:50.0,PV:40.5,Out:255
Set:50.0,PV:43.2,Out:230
```

### OLED Display Layout
```
┌──────────────────────┐
│TEMP. PID CONTROL    │┃
│PV: 48.5°            │┃
│Set:50  Pwr:85%      │█
│P:5.0 I:0.5 D:1.0    │█
└──────────────────────┘█
```

## การแก้ไขปัญหา (Troubleshooting)

### ปัญหา: จอ OLED ไม่แสดงผล
- ตรวจสอบการต่อสาย SDA (21) และ SCL (22)
- ลองสแกนหา I2C address: `i2cdetect -y 1` (หาก address ไม่ใช่ 0x3C ต้องแก้ในโค้ด)

### ปัญหา: เซ็นเซอร์อ่านค่า -127 หรือ 85°C
- ตรวจสอบ Pull-up resistor 4.7kΩ ที่ขา Data
- ตรวจสอบการต่อสาย VCC, GND และ Data
- ลองใช้สาย OneWire คุณภาพดีกว่า (สายสั้นกว่า)

### ปัญหา: Heater ไม่ทำงาน
- ตรวจสอบ MOSFET/SSR ต่อถูกต้อง (Gate → GPIO13)
- วัดแรงดันที่ Gate ต้องมีสัญญาณ PWM (ใช้ Oscilloscope หรือ LED)
- ตรวจสอบแหล่งจ่ายไฟของ Heater

### ปัญหา: ระบบ Oscillate มาก
- ลดค่า Kp
- ลดค่า Ki
- เพิ่มค่า Kd (เล็กน้อย)

## Safety Notes (ข้อควรระวัง)

⚠️ **คำเตือนด้านความปลอดภัย**:

1. **ไฟฟ้าแรงสูง**: ถ้าใช้ Heater ที่ต้องใช้ไฟ AC หรือแรงดันสูง ควรใช้ SSR และแยก Ground
2. **ความร้อน**: ติดตั้ง Heater ในพื้นที่ที่ปลอดภัย มีระบบระบายความร้อน
3. **ทดสอบ**: ทดสอบระบบในสภาพแวดล้อมที่ควบคุมได้ก่อนนำไปใช้งานจริง
4. **Overheat Protection**: โค้ดมีระบบป้องกันอุณหภูมิสูงเกินที่ 100°C แต่ควรติดตั้ง Thermal Fuse เพิ่มเติม
5. **ไฟดูด**: ESP32 + Heater อาจใช้กระแสสูง ควรใช้แหล่งจ่ายไฟที่เหมาะสม

## License

MIT License - ใช้งานและดัดแปลงได้อย่างอิสระ

## Contributing

ยินดีรับ Pull Requests และ Issues! 
- Report bugs
- Suggest features  
- Improve documentation
- Share your PID tuning results

## Author

THAITECHZONE - Thai Technology Community

## References

1. PID Control Theory: https://en.wikipedia.org/wiki/PID_controller
2. Ziegler-Nichols Method: https://en.wikipedia.org/wiki/Ziegler–Nichols_method
3. ESP32 PWM Documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
4. DS18B20 Datasheet: https://www.maximintegrated.com/en/products/sensors/DS18B20.html

---

**สนุกกับการทำโปรเจค! Happy Making! 🚀**
