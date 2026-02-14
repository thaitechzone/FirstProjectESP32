# ตัวอย่างการประยุกต์ใช้งาน (Application Examples)

คู่มือนี้แสดงตัวอย่างการปรับแต่งและประยุกต์ใช้ ESP32 PID Controller สำหรับงานต่างๆ

---

## 1. ตู้อบอุณหภูมิ (Drying Oven)

### Application Description
ใช้ควบคุมอุณหภูมิในตู้อบเพื่ออบแห้งวัสดุต่างๆ

### Hardware Modifications
```
- Heater: 500W Heating Element
- Enclosure: ตู้อบขนาด 30x30x30 cm
- Insulation: ฉนวนกันความร้อน
- Safety: Thermal Fuse 120°C
```

### PID Settings
```cpp
// ตู้อบมีมวลมาก ตอบสนองช้า
double Kp = 3.0;   // ไม่ต้องแรงมาก
double Ki = 0.3;   // ค่อยๆ ปรับ
double Kd = 0.8;   // Damping ปานกลาง

// ขยายขอบเขตอุณหภูมิ
#define TEMP_MIN 25.0
#define TEMP_MAX 120.0  // เพิ่มเป็น 120°C

Setpoint = 80.0;  // อบที่ 80°C
```

### Expected Performance
- Rise Time: 15-20 นาที
- Overshoot: < 2°C
- Steady-state Error: < 0.5°C

---

## 2. เครื่องชงกาแฟ Espresso (Coffee Machine)

### Application Description
ควบคุมอุณหภูมิน้ำสำหรับชงกาแฟที่ 92-96°C

### Hardware Modifications
```
- Heater: 1200W Heating Element
- Water Volume: 500ml
- Sensor Location: ในถังน้ำ
- Pressure Control: แยกระบบ
```

### PID Settings
```cpp
// ต้องการตอบสนองเร็ว และแม่นยำ
double Kp = 8.0;   // ตอบสนองเร็ว
double Ki = 1.2;   // กำจัด Error เร็ว
double Kd = 1.5;   // ป้องกัน Overshoot

#define TEMP_TARGET 94.0  // อุณหภูมิที่เหมาะสมที่สุด
Setpoint = TEMP_TARGET;

// PWM เร็วขึ้นสำหรับความแม่นยำ
const int PWM_FREQ = 5000;  // 5kHz
```

### Additional Code
```cpp
// เพิ่มการแจ้งเตือนเมื่อพร้อมชง
void loop() {
  // ... existing code ...
  
  // เช็คว่าถึงอุณหภูมิแล้ว
  if (abs(Input - Setpoint) < 1.0) {
    digitalWrite(LED_READY, HIGH);  // ไฟพร้อมชง
  }
}
```

### Expected Performance
- Rise Time: 3-5 นาที
- Overshoot: < 1°C
- Steady-state Error: < 0.3°C

---

## 3. ระบบเพาะเลี้ยงจุลินทรีย์ (Incubator)

### Application Description
ควบคุมอุณหภูมิคงที่ 37°C สำหรับเพาะเลี้ยงเชื้อ

### Hardware Modifications
```
- Heater: 100W Flexible Heating Pad
- Fan: 12V DC Fan สำหรับกระจายความร้อน
- Enclosure: ตู้ฉนวนกันความร้อนดี
```

### PID Settings
```cpp
// ต้องการความเสถียรสูงมาก
double Kp = 4.0;
double Ki = 0.4;
double Kd = 1.2;

#define TEMP_TARGET 37.0  // อุณหภูมิร่างกาย
Setpoint = TEMP_TARGET;

// ความแม่นยำสูง
#define TEMP_TOLERANCE 0.2  // ยอมให้ผิดเพียง ±0.2°C
```

### Additional Code
```cpp
// เพิ่มการควบคุมพัดลม
#define FAN_PIN 25

void setup() {
  // ... existing code ...
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
  // ... existing code ...
  
  // เปิดพัดลมเมื่อ Heater ทำงาน
  if (Output > 50) {
    digitalWrite(FAN_PIN, HIGH);  // เปิดพัดลม
  } else {
    digitalWrite(FAN_PIN, LOW);
  }
}
```

### Expected Performance
- Rise Time: 10-15 นาที
- Overshoot: < 0.5°C
- Steady-state Error: < 0.2°C

---

## 4. Hot Plate สำหรับ Reflow Soldering

### Application Description
ควบคุมอุณหภูมิตาม Reflow Profile สำหรับบัดกรี SMD

### Hardware Modifications
```
- Heater: Hot Plate 300W
- Size: 150x150mm
- Max Temp: 250°C
```

### PID Settings
```cpp
// ต้องการตอบสนองเร็วมาก
double Kp = 12.0;
double Ki = 2.0;
double Kd = 3.0;

#define TEMP_MAX 250.0  // เพิ่มเป็น 250°C
```

### Reflow Profile Implementation
```cpp
// ขั้นตอนของ Reflow Profile
enum ReflowStage {
  IDLE,
  PREHEAT,      // 150°C
  SOAK,         // 170°C
  REFLOW,       // 220°C
  COOLING
};

ReflowStage stage = IDLE;
unsigned long stageStartTime = 0;

void handleReflowProfile() {
  switch(stage) {
    case IDLE:
      Setpoint = 25.0;
      break;
      
    case PREHEAT:
      Setpoint = 150.0;
      // ถ้าถึงอุณหภูมิ และผ่าน 60 วินาที
      if (Input >= 145.0 && millis() - stageStartTime > 60000) {
        stage = SOAK;
        stageStartTime = millis();
      }
      break;
      
    case SOAK:
      Setpoint = 170.0;
      if (millis() - stageStartTime > 90000) {  // 90 วินาที
        stage = REFLOW;
        stageStartTime = millis();
      }
      break;
      
    case REFLOW:
      Setpoint = 220.0;
      if (Input >= 215.0 && millis() - stageStartTime > 30000) {
        stage = COOLING;
        stageStartTime = millis();
      }
      break;
      
    case COOLING:
      Setpoint = 50.0;
      ledcWrite(PWM_CHANNEL, 0);  // ปิด Heater
      if (Input < 60.0) {
        stage = IDLE;  // เสร็จสมบูรณ์
      }
      break;
  }
}

void loop() {
  // ... existing code ...
  handleReflowProfile();
  // ... rest of loop ...
}
```

### Expected Performance
- Rise Time: 2-3 นาที ต่อ stage
- Overshoot: < 3°C (ยอมรับได้)
- Profile Accuracy: < 5°C จาก Target

---

## 5. ระบบทำน้ำอุ่นพลังงานแสงอาทิตย์

### Application Description
ควบคุมอุณหภูมิน้ำในถังน้ำร้อน 200 ลิตร

### Hardware Modifications
```
- Main Heater: Solar Panel
- Backup Heater: 2000W Electric Element (ควบคุมด้วย SSR)
- Sensor: DS18B20 Waterproof แบบยาว
- Tank Size: 200 ลิตร
```

### PID Settings
```cpp
// มวลน้ำมากมาก ตอบสนองช้ามาก
double Kp = 1.0;   // อ่อนมาก
double Ki = 0.1;   // ค่อยๆ
double Kd = 0.3;   // เล็กน้อย

#define TEMP_TARGET 55.0  // น้ำอุ่น 55°C
Setpoint = TEMP_TARGET;

// PWM ช้าลงเพื่อป้องกัน SSR
const int PWM_FREQ = 10;  // 10Hz เท่านั้น
```

### Additional Features
```cpp
// ตรวจสอบพลังงานแสงอาทิตย์
#define SOLAR_SENSOR_PIN 36  // ADC

float getSolarPower() {
  int raw = analogRead(SOLAR_SENSOR_PIN);
  return raw / 4095.0 * 100.0;  // % of max
}

void loop() {
  // ... existing code ...
  
  float solarPower = getSolarPower();
  
  // ถ้ามีแสงอาทิตย์พอ ไม่ต้องใช้ Heater
  if (solarPower > 30.0 && Input > Setpoint - 5.0) {
    ledcWrite(PWM_CHANNEL, 0);  // ปิด Backup Heater
  }
  
  // แสดงสถานะบน OLED
  display.setCursor(0, 56);
  display.print("Sol:");
  display.print(solarPower, 0);
  display.print("%");
}
```

---

## 6. Sous Vide Cooker (ชุดปรุงอาหารอุณหภูมิต่ำ)

### Application Description
ควบคุมอุณหภูมิน้ำแม่นยำสูงสำหรับปรุงอาหาร

### Hardware Modifications
```
- Heater: 800W Immersion Heater
- Pump: 12V DC Water Pump (กระจายความร้อน)
- Container: 20 ลิตร
- Insulation: ฝาปิด + ฉนวน
```

### PID Settings
```cpp
// ต้องการความแม่นยำสูงมาก
double Kp = 6.0;
double Ki = 0.8;
double Kd = 2.0;

// ขอบเขตสำหรับอาหาร
#define TEMP_MIN 45.0
#define TEMP_MAX 90.0

// Recipes
const float TEMP_STEAK_RARE = 52.0;
const float TEMP_STEAK_MEDIUM = 57.0;
const float TEMP_CHICKEN = 65.0;
const float TEMP_FISH = 50.0;
```

### Recipe Selection
```cpp
// เพิ่มปุ่มเลือก Recipe
#define BTN_UP 32
#define BTN_DOWN 33
#define BTN_SELECT 25

int currentRecipe = 0;
const float recipes[] = {52.0, 57.0, 60.0, 65.0, 70.0};
const char* recipeNames[] = {"Rare", "Med", "Well", "Chick", "Pork"};

void setup() {
  // ... existing code ...
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
}

void handleButtons() {
  if (digitalRead(BTN_UP) == LOW) {
    currentRecipe = (currentRecipe + 1) % 5;
    delay(200);  // debounce
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    currentRecipe = (currentRecipe - 1 + 5) % 5;
    delay(200);
  }
  if (digitalRead(BTN_SELECT) == LOW) {
    Setpoint = recipes[currentRecipe];
    delay(200);
  }
}

void loop() {
  handleButtons();
  // ... existing code ...
  
  // แสดง Recipe บน OLED
  display.setCursor(0, 48);
  display.print("Recipe: ");
  display.print(recipeNames[currentRecipe]);
}
```

### Expected Performance
- Rise Time: 20-30 นาที
- Temperature Accuracy: ±0.1°C
- Steady-state Error: < 0.1°C

---

## 7. PCB Preheater

### Application Description
ให้ความร้อนล่างสำหรับงาน Rework/Soldering

### PID Settings
```cpp
double Kp = 10.0;  // ตอบสนองเร็ว
double Ki = 1.5;
double Kd = 2.5;

#define TEMP_PREHEAT 120.0  // Preheat ที่ 120°C
Setpoint = TEMP_PREHEAT;
```

### Safety Timer
```cpp
// ปิดอัตโนมัติหลัง 30 นาที
unsigned long heaterOnTime = 0;
const unsigned long MAX_ON_TIME = 30 * 60 * 1000;  // 30 min

void loop() {
  // ... existing code ...
  
  if (Output > 0) {
    if (heaterOnTime == 0) heaterOnTime = millis();
    
    // ถ้าเปิดเกิน 30 นาที
    if (millis() - heaterOnTime > MAX_ON_TIME) {
      ledcWrite(PWM_CHANNEL, 0);
      displayError("TIMEOUT", "30 MIN");
      return;
    }
  } else {
    heaterOnTime = 0;
  }
}
```

---

## 8. 3D Printer Heated Bed

### Application Description
ควบคุมอุณหภูมิ Heated Bed สำหรับ 3D Printer

### PID Settings
```cpp
// สำหรับ Aluminum Heated Bed
double Kp = 7.0;
double Ki = 0.7;
double Kd = 1.8;

// อุณหภูมิตาม Material
const float TEMP_PLA = 60.0;
const float TEMP_ABS = 100.0;
const float TEMP_PETG = 80.0;
```

### Integration with Printer
```cpp
// รับคำสั่งผ่าน Serial (G-code)
void processGcode() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    
    // M140 S60 - Set bed temp
    if (cmd.startsWith("M140 S")) {
      float temp = cmd.substring(7).toFloat();
      if (temp >= TEMP_MIN && temp <= TEMP_MAX) {
        Setpoint = temp;
        Serial.println("ok");
      }
    }
    
    // M105 - Get current temp
    if (cmd.startsWith("M105")) {
      Serial.print("ok T:");
      Serial.print(Input);
      Serial.print(" B:");
      Serial.println(Setpoint);
    }
  }
}
```

---

## สรุปตารางเปรียบเทียบ

| Application | Kp | Ki | Kd | Rise Time | Accuracy |
|-------------|----|----|----|-----------|----|
| Drying Oven | 3.0 | 0.3 | 0.8 | 15-20 min | ±0.5°C |
| Coffee Machine | 8.0 | 1.2 | 1.5 | 3-5 min | ±0.3°C |
| Incubator | 4.0 | 0.4 | 1.2 | 10-15 min | ±0.2°C |
| Reflow | 12.0 | 2.0 | 3.0 | 2-3 min | ±3°C |
| Water Heater | 1.0 | 0.1 | 0.3 | 60+ min | ±1°C |
| Sous Vide | 6.0 | 0.8 | 2.0 | 20-30 min | ±0.1°C |
| PCB Preheater | 10.0 | 1.5 | 2.5 | 5-8 min | ±1°C |
| 3D Printer Bed | 7.0 | 0.7 | 1.8 | 5-10 min | ±0.5°C |

---

## การปรับแต่งทั่วไป

### กฎทั่วไปสำหรับเลือกค่า PID:

1. **ระบบมวลน้อย ตอบสนองเร็ว** → Kp สูง, Ki สูง, Kd สูง
2. **ระบบมวลมาก ตอบสนองช้า** → Kp ต่ำ, Ki ต่ำ, Kd ต่ำ
3. **ต้องการความแม่นยำสูง** → เพิ่ม Kd, ลด Kp
4. **ต้องการตอบสนองเร็ว** → เพิ่ม Kp, Ki
5. **มี Noise มาก** → ลด Kd

---

ทุกตัวอย่างสามารถนำไปประยุกต์ใช้โดยแก้ไขจาก `main.cpp` หลักได้ครับ! 🚀
