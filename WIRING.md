# Wiring Diagram - ESP32 PID Temperature Controller

## แผนผังการต่อสายอย่างละเอียด (Detailed Wiring Diagram)

```
                                    ESP32 DevKit V2
                        ╔═══════════════════════════════════════╗
                        ║  ┌─────────────────────────────┐     ║
                        ║  │    USB        ESP32-WROOM  │     ║
         USB Cable ─────╫──┤                              ├────╫──── 3.3V ────┐
                        ║  └─────────────────────────────┘     ║              │
                        ║                                       ║              │
                        ║  GPIO Pins:                           ║              │
                        ║                                       ║              │
    Heater Control <────╫── GPIO 13 (PWM)                      ║              │
    DS18B20 Data   <────╫── GPIO 14 (OneWire)                  ║              │
    OLED SDA       <────╫── GPIO 21 (I2C)                      ║              │
    OLED SCL       <────╫── GPIO 22 (I2C)                      ║              │
                        ║                                       ║              │
    Common GND     <────╫── GND                                ║              │
                        ╚═══════════════════════════════════════╝              │
                                                                               │
┌──────────────────────────────────────────────────────────────────────────┐ │
│                        COMPONENT CONNECTIONS                              │ │
└──────────────────────────────────────────────────────────────────────────┘ │
                                                                               │
┌─────────────────────────────────────────────────────────────┐              │
│ 1. DS18B20 Temperature Sensor                               │              │
│    (Waterproof version recommended)                         │              │
└─────────────────────────────────────────────────────────────┘              │
                                                                               │
        DS18B20 (Flat side facing you)                                        │
        ┌─────────────┐                                                       │
        │   ┌─┬─┬─┐   │                                                       │
        │   │1│2│3│   │                                                       │
        └───┴─┴─┴─┴───┘                                                       │
            │ │ │                                                              │
            │ │ └────────────────────────── VCC (3.3V) <─────────────────────┘
            │ │                    ┌────── 4.7kΩ Pull-up
            │ └────────────────────┴──────── GPIO 14
            └────────────────────────────── GND


┌─────────────────────────────────────────────────────────────┐
│ 2. OLED Display (0.96" I2C 128x64)                          │
└─────────────────────────────────────────────────────────────┘

        OLED Module (4-pin I2C)
        ┌─────────────────┐
        │   ┌─────────┐   │
        │   │ OLED    │   │
        │   │ Display │   │
        │   └─────────┘   │
        │  GND VCC SCL SDA│
        └───┬───┬───┬───┬─┘
            │   │   │   │
            │   │   │   └───────────────── GPIO 21 (SDA)
            │   │   └───────────────────── GPIO 22 (SCL)
            │   └───────────────────────── 3.3V
            └───────────────────────────── GND


┌─────────────────────────────────────────────────────────────┐
│ 3. Heater Driver Circuit (MOSFET)                           │
│    Recommended: IRLZ44N or similar Logic-Level MOSFET      │
└─────────────────────────────────────────────────────────────┘

                                    Heater Power Supply
                                           +12V (or appropriate)
                                            │
                                            │
                                         ┌──┴──┐
                    Heater Element       │     │
                    (Resistive Load)     │  H  │
                                         │  E  │
                                         │  A  │
                                         │  T  │
                                         │  E  │
                                         │  R  │
                                         └──┬──┘
                                            │
                                            │ Drain (D)
                                         ┌──┴──┐
                                         │     │
     GPIO 13 (PWM) ──────── 220Ω ───────┤Gate │ MOSFET
                                         │     │ IRLZ44N
     GND (ESP32) ────────────────────────┤Source
                                         └─────┘
                                            │
                                            │
                                          GND (Heater PSU)

    Notes:
    - 220Ω resistor protects GPIO pin
    - Use heatsink on MOSFET if high current
    - Ensure common ground between ESP32 and Heater PSU


┌─────────────────────────────────────────────────────────────┐
│ 4. Alternative: Solid State Relay (SSR) for AC Heater      │
└─────────────────────────────────────────────────────────────┘

                                    AC Mains (110V/220V)
                                         Live ─┐
                                              │
                                           ┌──┴──┐
                       AC Heater           │     │
                                           └──┬──┘
                                              │
                                         ┌────┴────┐
     GPIO 13 (PWM) ──── 220Ω ────┬──────┤ SSR     │
                                  │      │ Input   │
     GND (ESP32) ─────────────────┴──────┤   +  -  │
                                         └────┬────┘
                                              │
                                         Neutral ─┘

    ⚠️ HIGH VOLTAGE WARNING:
    - Only qualified electricians should work with AC mains
    - Use proper isolation and enclosure
    - Test with low voltage DC first


┌─────────────────────────────────────────────────────────────┐
│ Complete Wiring Summary                                     │
└─────────────────────────────────────────────────────────────┘

    Connection Table:
    ┌──────────────┬─────────────────────┬────────────────────┐
    │ ESP32 Pin    │ Connected To        │ Notes              │
    ├──────────────┼─────────────────────┼────────────────────┤
    │ GPIO 13      │ Heater Gate/SSR     │ PWM Output         │
    │ GPIO 14      │ DS18B20 Data        │ Needs 4.7kΩ pullup │
    │ GPIO 21      │ OLED SDA            │ I2C Data           │
    │ GPIO 22      │ OLED SCL            │ I2C Clock          │
    │ 3.3V         │ DS18B20, OLED VCC   │ Max 500mA total    │
    │ GND          │ All Component GNDs  │ Common Ground      │
    └──────────────┴─────────────────────┴────────────────────┘

    Power Requirements:
    ┌──────────────┬─────────────┬────────────────────────┐
    │ Component    │ Voltage     │ Current                │
    ├──────────────┼─────────────┼────────────────────────┤
    │ ESP32        │ 5V (USB)    │ 500mA (peak)           │
    │ DS18B20      │ 3.3V        │ 1mA (active)           │
    │ OLED Display │ 3.3V        │ 20mA (typical)         │
    │ Heater       │ Varies      │ Depends on element     │
    └──────────────┴─────────────┴────────────────────────┘


┌─────────────────────────────────────────────────────────────┐
│ Bill of Materials (BOM)                                     │
└─────────────────────────────────────────────────────────────┘

    Essential Components:
    ☐ 1x ESP32 DevKit V2 Board
    ☐ 1x DS18B20 Temperature Sensor (waterproof recommended)
    ☐ 1x 4.7kΩ Resistor (Pull-up for DS18B20)
    ☐ 1x OLED Display 0.96" I2C (128x64, SSD1306)
    ☐ 1x MOSFET (IRLZ44N) or SSR (for AC)
    ☐ 1x 220Ω Resistor (Gate resistor)
    ☐ 1x Heater Element (appropriate for application)
    ☐ Jumper wires / Breadboard
    ☐ Power supplies (5V for ESP32, appropriate for heater)

    Optional (Recommended):
    ☐ PCB or Prototyping board
    ☐ Heatsink for MOSFET
    ☐ Thermal fuse (safety)
    ☐ Enclosure box
    ☐ Screw terminals
    ☐ LED indicators


┌─────────────────────────────────────────────────────────────┐
│ Testing Checklist                                           │
└─────────────────────────────────────────────────────────────┘

    Before powering on:
    ☐ Verify all connections match the diagram
    ☐ Check for short circuits with multimeter
    ☐ Ensure correct polarity on all components
    ☐ Verify pull-up resistor is in place
    ☐ Check I2C address of OLED (usually 0x3C)
    
    Initial power-up (without heater):
    ☐ Connect ESP32 via USB only
    ☐ Upload and monitor via Serial (115200 baud)
    ☐ Verify OLED displays startup screen
    ☐ Verify DS18B20 reads room temperature
    ☐ Check PWM output with LED on GPIO 13
    
    With heater connected:
    ☐ Start with low setpoint (30°C)
    ☐ Monitor temperature rise
    ☐ Verify safety cutoff at 100°C works
    ☐ Test manual setpoint changes via Serial
    ☐ Observe PID behavior and tune if needed
