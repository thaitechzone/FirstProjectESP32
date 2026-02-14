# ESP32 PID Temperature Controller - State Machine Implementation

## สรุปการพัฒนา

โปรเจคนี้ได้รับการปรับปรุงจากระบบควบคุมอุณหภูมิ PID แบบเดิมให้เป็น **State Machine Architecture** ที่มีโหมดการทำงานที่หลากหลายและใช้งานง่ายขึ้น

### การเปลี่ยนแปลงหลัก

#### ✅ ระบบ State Machine
- เพิ่ม **5 States** สำหรับการทำงานที่แตกต่างกัน
- State transition ที่ชัดเจนและเป็นระเบียบ
- แยก concerns ของแต่ละโหมดออกจากกัน

#### ✅ Button Controls
- **SW1 (GPIO 34)**: Mode/Enter - สลับโหมด
- **SW2 (GPIO 35)**: Down - ลดค่า
- **SW3 (GPIO 32)**: Up - เพิ่มค่า
- Debouncing 200ms เพื่อป้องกันการกดซ้ำ

#### ✅ โหมดการทำงาน (States)

1. **State_Processing** (เริ่มต้น)
   - ควบคุม PID อัตโนมัติ
   - Safety checks (sensor error, overheat)
   - แสดงข้อมูลครบถ้วน

2. **State_SetPoint_Config**
   - ตั้งค่า Setpoint ด้วยปุ่ม Up/Down
   - ช่วง: 25-100°C

3. **State_PID_Config**
   - ตั้งค่า Kp, Ki, Kd
   - เลือก parameter ด้วย SW1
   - ปรับค่าด้วย Up/Down

4. **State_Manual_PWM**
   - ทดสอบ PWM โดยตรง (0-255)
   - ไม่ใช้ PID
   - แสดงอุณหภูมิแบบ Real-time

5. **State_Display**
   - สำรองไว้ใช้ในอนาคต

### เอกสารประกอบ

📖 **[STATE_MACHINE_GUIDE.md](STATE_MACHINE_GUIDE.md)** - คู่มือการใช้งานแบบละเอียด
- State flow diagram
- วิธีใช้แต่ละโหมด
- คำแนะนำ PID Tuning
- Troubleshooting

### โครงสร้างโค้ด

```
src/main.cpp
├── Header & Documentation (บรรทัด 1-40)
├── Pin Definitions (บรรทัด 19-36)
├── State Machine Definition (บรรทัด 70-90)
├── setup() - Button initialization
├── loop() - State dispatcher
├── handleButtons() - Button logic & state transitions
├── State Functions:
│   ├── stateProcessing()
│   ├── stateSetPointConfig()
│   ├── statePIDConfig()
│   ├── stateManualPWM()
│   └── stateDisplay()
└── Helper Functions (updateDisplay, readButton, etc.)
```

### การรักษา Backward Compatibility

✅ **ฟังก์ชันเดิมทั้งหมดยังคงทำงานได้**
- Safety checks (sensor error, overheat)
- Serial communication สำหรับตั้งค่า Setpoint
- Display functions
- PID control loop

### Statistics

- **Lines Added**: ~240 lines
- **New States**: 5 states
- **New Functions**: 6 functions (5 state + 1 button handler)
- **Documentation**: 160+ lines in guide

### การใช้งาน

```bash
# Build & Upload
platformio run --target upload

# Monitor Serial
platformio device monitor
```

### ทดสอบการทำงาน

เมื่อเปิดเครื่อง:
1. เริ่มที่ **State_Processing** (แสดงหน้าจอปกติ)
2. กด **SW1** → ไป SetPoint Config
3. ปรับ Setpoint ด้วย **SW3/SW2**
4. กด **SW1** → ไป PID Config
5. เลือก parameter ด้วย **SW1**, ปรับค่าด้วย **SW3/SW2**
6. กด **SW1** ครบ 4 ครั้ง → ไป Manual PWM
7. ทดสอบ PWM ด้วย **SW3/SW2**
8. กด **SW1** → กลับ Processing

### ความปลอดภัย

🛡️ **Safety Features ทั้งหมดยังคงทำงานในทุก State:**
- Auto-cutoff เมื่อ sensor error
- Auto-cutoff เมื่ออุณหภูมิ > 100°C
- Setpoint limits (25-100°C)
- Parameter validation (>= 0)

### Author

THAITECHZONE
- ESP32 DevKit V2 Board
- PID Temperature Control System

### License

ใช้งานได้เฉพาะในการศึกษาและพัฒนา
