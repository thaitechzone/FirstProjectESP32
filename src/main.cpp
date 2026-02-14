/**
 * ESP32 PID Temperature Controller (Professional Edition with State Machine)
 * ---------------------------------------------------------------------------
 * รายละเอียด Hardware:
 * - MCU: ESP32 DevKit V2 Board (THAITECHZONE)
 * - Sensor: DS18B20 (ต่อที่ GPIO 14)
 * - Heater: MOSFET หรือ SSR (ต่อที่ GPIO 13) -> ควบคุมด้วย PWM
 * - Display: OLED 0.96" (I2C: SDA=21, SCL=22)
 * - Buttons: SW1(GPIO 34), SW2(GPIO 35), SW3(GPIO 32)
 * 
 * State Machine:
 * --------------
 * 1. State_Processing (เริ่มต้น) - ควบคุม PID อัตโนมัติ
 *    - แสดงอุณหภูมิปัจจุบัน, Setpoint, กำลังงาน, และค่า PID
 *    - กด SW1 เพื่อเข้าโหมดตั้งค่า
 * 
 * 2. State_SetPoint_Config - ตั้งค่า Setpoint
 *    - กด SW3 (Up) เพิ่มค่า Setpoint
 *    - กด SW2 (Down) ลดค่า Setpoint
 *    - กด SW1 เพื่อไปโหมด PID Config
 * 
 * 3. State_PID_Config - ตั้งค่า P, I, D
 *    - กด SW1 เพื่อเลือก parameter (Kp/Ki/Kd)
 *    - กด SW3 (Up) เพิ่มค่า parameter ที่เลือก
 *    - กด SW2 (Down) ลดค่า parameter ที่เลือก
 *    - กด SW1 เพื่อไปโหมด Manual PWM
 * 
 * 4. State_Manual_PWM - ทดสอบ PWM แบบ Manual
 *    - กด SW3 (Up) เพิ่มค่า PWM
 *    - กด SW2 (Down) ลดค่า PWM
 *    - กด SW1 เพื่อกลับโหมด Processing
 * 
 * 5. State_Display - (สำรองไว้ใช้ในอนาคต)
 *    - State สำหรับแสดงผลอย่างเดียว
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

// =========================================
// 1. การกำหนดขาพอร์ต (Pin Definitions)
// =========================================
#define HEATER_PIN 13    // ขาจ่ายสัญญาณ PWM ไปยัง Heater Driver
#define ONE_WIRE_BUS 14  // ขา Data ของ Sensor DS18B20

// Manual Control IO (สำรองไว้ใช้ในอนาคต)
#define SW1_PIN 34       // สวิตช์ 1 (Input Only - ต้องต่อ R Pull-up 10k)
#define SW2_PIN 35       // สวิตช์ 2 (Input Only - ต้องต่อ R Pull-up 10k)
#define SW3_PIN 32       // สวิตช์ 3 (มี Internal Pull-up)
#define RL1_PIN 17       // รีเลย์ 1
#define RL2_PIN 16       // รีเลย์ 2
#define RL3_PIN 4        // รีเลย์ 3

// Isolated Inputs (สำรองไว้ใช้ในอนาคต)
#define ISOIN1_PIN 33
#define ISOIN2_PIN 27

// =========================================
// 2. การตั้งค่าระบบ (System Settings)
// =========================================
// ขอบเขตอุณหภูมิและความปลอดภัย
#define TEMP_MIN 25.0    // ค่าต่ำสุดที่ยอมให้ตั้ง
#define TEMP_MAX 100.0    // ค่าสูงสุด และจุดตัด Safety Cutoff

// การตั้งค่า PWM (สำหรับ ESP32)
const int PWM_FREQ = 1000;     // ความถี่ 1kHz (เหมาะกับ MOSFET)
const int PWM_CHANNEL = 0;     // ช่องสัญญาณ PWM 0
const int PWM_RESOLUTION = 8;  // ความละเอียด 8-bit (ค่า 0-255)

// =========================================
// 3. ประกาศตัวแปรและ Object
// =========================================
// ตั้งค่าจอ OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ตั้งค่า Sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ตัวแปร PID
double Setpoint, Input, Output;
// ค่า Tuning (ปรับจูนตามความเหมาะสมของระบบจริง)
double Kp = 5.0, Ki = 0.5, Kd = 1.0; 
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// ตัวแปรจับเวลา
unsigned long lastDisplayTime = 0;

// =========================================
// 4. State Machine Definition
// =========================================
enum SystemState {
  State_Display,           // อัพเดทการแสดงผล
  State_Processing,        // ประมวลผล PID หลัก
  State_SetPoint_Config,   // ตั้งค่า Setpoint
  State_PID_Config,        // ตั้งค่า P, I, D
  State_Manual_PWM         // ทดสอบ PWM แบบ Manual
};

SystemState currentState = State_Processing;  // เริ่มต้นที่โหมดประมวลผล

// ตัวแปรสำหรับ Config Menu
int configIndex = 0;          // ใช้เลือกเมนู (0=P, 1=I, 2=D)
int manualPWM = 0;            // ค่า PWM สำหรับโหมด Manual (0-255)
unsigned long sw1PressTime = 0;  // เวลากดปุ่ม SW1

// ตัวแปรสำหรับ Button Debouncing
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 200;  // 200ms

// ประกาศชื่อฟังก์ชันล่วงหน้า (Function Prototypes)
void updateDisplay();
void displayError(String title, String msg);
void debugSerial();
bool readButton(int pin);
void handleButtons();
void stateDisplay();
void stateProcessing();
void stateSetPointConfig();
void statePIDConfig();
void stateManualPWM();

void setup() {
  Serial.begin(115200);
  
  // --- A. ตั้งค่า PWM สำหรับ Heater ---
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(HEATER_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // เริ่มต้นปิด Heater

  // --- B. เริ่มต้น Sensor ---
  sensors.begin();

  // --- C. เริ่มต้นจอ OLED ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // ถ้าจอเสีย ให้หยุดทำงานตรงนี้
  }
  
  // แสดง Logo เริ่มต้น
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println(F("SYSTEM STARTING..."));
  display.setCursor(10, 40);
  display.println(F("PWM PID CONTROL"));
  display.display();
  delay(1500);

  // --- D. เริ่มต้น PID ---
  Setpoint = 40.0; // ตั้งค่าเริ่มต้นที่ 40 องศา
  
  // กำหนดขอบเขต Output ให้ตรงกับ PWM (0-255)
  myPID.SetOutputLimits(0, 255);
  myPID.SetMode(AUTOMATIC);

  // --- E. ตั้งค่าปุ่ม (Button Setup) ---
  pinMode(SW1_PIN, INPUT);  // Mode/Enter button
  pinMode(SW2_PIN, INPUT);  // Down button  
  pinMode(SW3_PIN, INPUT_PULLUP);  // Up button (มี internal pull-up)

  Serial.println(F("--- ESP32 PID Ready ---"));
  Serial.println(F("SW1=Mode/Enter | SW2=Down | SW3=Up"));
  Serial.println(F("Type temp in Serial (e.g., 60.5) to change Setpoint"));
}

void loop() {
  // จัดการปุ่มกด
  handleButtons();
  
  // ประมวลผลตาม State ปัจจุบัน
  switch (currentState) {
    case State_Display:
      stateDisplay();
      break;
    case State_Processing:
      stateProcessing();
      break;
    case State_SetPoint_Config:
      stateSetPointConfig();
      break;
    case State_PID_Config:
      statePIDConfig();
      break;
    case State_Manual_PWM:
      stateManualPWM();
      break;
  }
}

void updateDisplay() {
  display.clearDisplay();

  // ส่วนหัว
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(F("TEMP. PID CONTROL"));

  // แสดง PV (ตัวใหญ่)
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(F("PV:"));
  display.print(Input, 1); 

  // แสดง Setpoint และ % กำลังไฟ
  display.setTextSize(1);
  display.setCursor(0, 36);
  display.print(F("Set:")); display.print(Setpoint, 0);
  display.print(F(" Pwr:")); 
  display.print(map(Output, 0, 255, 0, 100)); // แปลงเป็น %
  display.print(F("%"));

  // แสดงค่า PID Tuning (Kp, Ki, Kd)
  display.setCursor(0, 48);
  display.print(F("P:")); display.print(Kp, 1);
  display.print(F(" I:")); display.print(Ki, 1);
  display.print(F(" D:")); display.print(Kd, 1);

  // กราฟแท่งแสดงกำลังไฟ Heater (เต็มหน้าจอ ชิดขวา)
  int barHeight = map(Output, 0, 255, 0, 64);
  display.drawRect(118, 0, 10, 64, WHITE); // กรอบเต็มความสูง
  display.fillRect(118, 64 - barHeight, 10, barHeight, WHITE); // ไส้ใน

  display.display();
}

void displayError(String title, String msg) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.println(title);
  
  display.setTextSize(2);
  display.setCursor(5, 35);
  display.print(F("!! "));
  display.print(msg);
  
  display.display();
}

void debugSerial() {
  // รูปแบบข้อมูลสำหรับ Serial Plotter: SP, PV, Output
  Serial.print("Set:"); Serial.print(Setpoint); Serial.print(",");
  Serial.print("PV:"); Serial.print(Input); Serial.print(",");
  Serial.print("Out:"); Serial.println(Output);
}

// =========================================
// 5. Button Functions
// =========================================
bool readButton(int pin) {
  // อ่านปุ่มพร้อม debounce
  // Return true ถ้ามีการกดปุ่ม (active low)
  if (millis() - lastButtonTime < debounceDelay) {
    return false;  // ยังไม่ถึงเวลา debounce
  }
  
  bool pressed = (digitalRead(pin) == LOW);
  if (pressed) {
    lastButtonTime = millis();
  }
  return pressed;
}

void handleButtons() {
  // ปุ่ม SW1 (Mode/Enter) - สลับ State
  if (readButton(SW1_PIN)) {
    // ถ้าอยู่ใน PID Config Mode ให้สลับ parameter
    if (currentState == State_PID_Config) {
      configIndex = (configIndex + 1) % 3;  // วน 0->1->2->0
      Serial.print(F("Select Parameter: "));
      if (configIndex == 0) Serial.println(F("Kp"));
      else if (configIndex == 1) Serial.println(F("Ki"));
      else Serial.println(F("Kd"));
    } else {
      // สลับ State ปกติ
      switch(currentState) {
        case State_Processing:
          currentState = State_SetPoint_Config;
          Serial.println(F("Mode: SetPoint Config"));
          break;
        case State_SetPoint_Config:
          currentState = State_PID_Config;
          configIndex = 0;
          Serial.println(F("Mode: PID Config"));
          break;
        case State_PID_Config:
          currentState = State_Manual_PWM;
          manualPWM = 0;
          Serial.println(F("Mode: Manual PWM"));
          break;
        case State_Manual_PWM:
          currentState = State_Processing;
          Serial.println(F("Mode: Processing"));
          break;
        default:
          currentState = State_Processing;
          break;
      }
    }
    delay(10);  // Small delay after state change
  }
}

// =========================================
// 6. State Functions
// =========================================

void stateDisplay() {
  // State สำหรับแสดงผลอย่างเดียว (ไม่ได้ใช้ใน loop หลัก แต่สามารถเรียกใช้ได้)
  if (millis() - lastDisplayTime > 200) {
    updateDisplay();
    lastDisplayTime = millis();
  }
}

void stateProcessing() {
  // State หลัก: ประมวลผล PID และควบคุมอุณหภูมิ
  
  // 1. รับคำสั่งเปลี่ยนอุณหภูมิผ่าน Serial
  if (Serial.available() > 0) {
    float newSp = Serial.parseFloat();
    while(Serial.available()) Serial.read(); // เคลียร์ Buffer
    
    if (newSp >= TEMP_MIN && newSp <= TEMP_MAX) {
      Setpoint = newSp;
      Serial.print(F("New Setpoint: ")); Serial.println(Setpoint);
    } else if (newSp > 0) {
      Serial.println(F("Error: Temp out of range!"));
    }
  }

  // 2. อ่านค่าอุณหภูมิ
  sensors.requestTemperatures(); 
  double currentTemp = sensors.getTempCByIndex(0);

  // 3. ตรวจสอบความปลอดภัย (Safety Checks)
  
  // กรณี 3.1: Sensor มีปัญหา (ค่า -127 หรือ 85)
  if (currentTemp == -127.00 || currentTemp == 85.00) {
    ledcWrite(PWM_CHANNEL, 0); // ตัด Heater ทันที
    displayError("SENSOR", "ERROR");
    return; 
  }
  
  Input = currentTemp;

  // กรณี 3.2: อุณหภูมิเกินกำหนด (Overheat)
  if (Input > TEMP_MAX) {
    ledcWrite(PWM_CHANNEL, 0); // ตัด Heater ทันที
    Output = 0;
    displayError("OVERHEAT", "> 100C");
    Serial.println(F("ALARM: Overheat detected!"));
    return;
  }

  // 4. คำนวณ PID
  myPID.Compute();

  // 5. ส่งค่าไปยัง Hardware (PWM Output)
  ledcWrite(PWM_CHANNEL, (int)Output);

  // 6. แสดงผล (ทุกๆ 200ms)
  if (millis() - lastDisplayTime > 200) {
    updateDisplay();
    debugSerial();
    lastDisplayTime = millis();
  }
}

void stateSetPointConfig() {
  // State สำหรับตั้งค่า Setpoint ด้วยปุ่ม Up/Down
  
  // อ่านปุ่ม Up (SW3) และ Down (SW2)
  if (readButton(SW3_PIN)) {
    Setpoint += 1.0;
    if (Setpoint > TEMP_MAX) Setpoint = TEMP_MAX;
    Serial.print(F("Setpoint: ")); Serial.println(Setpoint);
  }
  
  if (readButton(SW2_PIN)) {
    Setpoint -= 1.0;
    if (Setpoint < TEMP_MIN) Setpoint = TEMP_MIN;
    Serial.print(F("Setpoint: ")); Serial.println(Setpoint);
  }
  
  // อ่านค่าอุณหภูมิ (เพื่อแสดงผล)
  sensors.requestTemperatures(); 
  Input = sensors.getTempCByIndex(0);
  
  // แสดงผลแบบ Config Mode
  if (millis() - lastDisplayTime > 200) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("CONFIG: SETPOINT"));
    
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.print(F("SP: "));
    display.print(Setpoint, 1);
    
    display.setTextSize(1);
    display.setCursor(0, 38);
    display.print(F("PV: "));
    display.print(Input, 1);
    
    display.setCursor(0, 52);
    display.print(F("UP/DN: +/-  SW1:Next"));
    
    display.display();
    lastDisplayTime = millis();
  }
}

void statePIDConfig() {
  // State สำหรับตั้งค่า P, I, D ด้วยปุ่ม
  
  // ปุ่ม Up/Down เพื่อปรับค่า
  double increment = 0.1;
  
  if (readButton(SW3_PIN)) {
    switch(configIndex) {
      case 0: Kp += increment; break;
      case 1: Ki += increment; break;
      case 2: Kd += increment; break;
    }
    myPID.SetTunings(Kp, Ki, Kd);
    Serial.print(F("P:")); Serial.print(Kp);
    Serial.print(F(" I:")); Serial.print(Ki);
    Serial.print(F(" D:")); Serial.println(Kd);
  }
  
  if (readButton(SW2_PIN)) {
    switch(configIndex) {
      case 0: 
        Kp -= increment; 
        if (Kp < 0) Kp = 0;
        break;
      case 1: 
        Ki -= increment; 
        if (Ki < 0) Ki = 0;
        break;
      case 2: 
        Kd -= increment; 
        if (Kd < 0) Kd = 0;
        break;
    }
    myPID.SetTunings(Kp, Ki, Kd);
    Serial.print(F("P:")); Serial.print(Kp);
    Serial.print(F(" I:")); Serial.print(Ki);
    Serial.print(F(" D:")); Serial.println(Kd);
  }
  
  // แสดงผลแบบ Config Mode
  if (millis() - lastDisplayTime > 200) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("CONFIG: PID TUNING"));
    
    display.setTextSize(1);
    display.setCursor(0, 16);
    if (configIndex == 0) display.print(F(">"));
    display.print(F(" Kp: ")); display.println(Kp, 2);
    
    display.setCursor(0, 28);
    if (configIndex == 1) display.print(F(">"));
    display.print(F(" Ki: ")); display.println(Ki, 2);
    
    display.setCursor(0, 40);
    if (configIndex == 2) display.print(F(">"));
    display.print(F(" Kd: ")); display.println(Kd, 2);
    
    display.setCursor(0, 54);
    display.print(F("SW1:Sel UP/DN:+/-"));
    
    display.display();
    lastDisplayTime = millis();
  }
}

void stateManualPWM() {
  // State สำหรับทดสอบ PWM แบบ Manual
  
  // ปุ่ม Up/Down เพื่อปรับ PWM
  if (readButton(SW3_PIN)) {
    manualPWM += 10;
    if (manualPWM > 255) manualPWM = 255;
    Serial.print(F("Manual PWM: ")); Serial.println(manualPWM);
  }
  
  if (readButton(SW2_PIN)) {
    manualPWM -= 10;
    if (manualPWM < 0) manualPWM = 0;
    Serial.print(F("Manual PWM: ")); Serial.println(manualPWM);
  }
  
  // ส่งค่า PWM โดยตรง (ไม่ผ่าน PID)
  ledcWrite(PWM_CHANNEL, manualPWM);
  
  // อ่านค่าอุณหภูมิ (เพื่อแสดงผล)
  sensors.requestTemperatures(); 
  Input = sensors.getTempCByIndex(0);
  
  // แสดงผลแบบ Manual Mode
  if (millis() - lastDisplayTime > 200) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("MANUAL PWM TEST"));
    
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.print(F("PWM:"));
    display.print(manualPWM);
    
    display.setTextSize(1);
    display.setCursor(0, 36);
    display.print(F("Power: "));
    display.print(map(manualPWM, 0, 255, 0, 100));
    display.print(F("%"));
    
    display.setCursor(0, 48);
    display.print(F("Temp: "));
    display.print(Input, 1);
    display.print(F("C"));
    
    display.setCursor(0, 56);
    display.print(F("UP/DN:+/-  SW1:Exit"));
    
    display.display();
    lastDisplayTime = millis();
  }
}