Radix Master v1.1 — Open Source STM32 STEM Learning Console | 12 Games | Binary · DNA · Chemistry
Hands-on educational hardware console teaching Digital Logic, Molecular Biology and Chemistry through interactive switch-based gameplay

## Project Overview

Radix Master v1.1 is a self-contained, hardware-based educational system powered by the STM32 Blue Pill microcontroller. It is designed to bridge the gap between abstract academic theory and genuine conceptual understanding by converting curriculum topics into 
timed, tactile problem-solving challenges.
Students interact with an 8-bit DIP switch array, a rotary encoder, a 20×4 LCD, and dual 7-segment displays — moving learning away from passive screen interaction toward deliberate, physical engagement. The system is fully open-source and customizable for classroom deployment.

━━━━━━━━━━━━━━━━━
## Educational Objectives
━━━━━━━━━━━━━━━━━

This console directly supports learning outcomes in:

COMPUTER SCIENCE
- Binary and hexadecimal number representation
- Bitwise operations: AND, OR, XOR, NAND, NOR, XNOR, NOT
- Base conversion across decimal, binary, and hexadecimal
- Binary arithmetic: addition, subtraction, 
  multiplication, division

MATHEMATICS
- Prime number theory and identification
- Pattern recognition under time pressure

COMMUNICATION TECHNOLOGY  
- Morse code signal encoding and timing

CHEMISTRY & PHYSICS
- Electron shell configuration
- Valence electron identification across 118 elements
- Periodic table navigation

MOLECULAR BIOLOGY
- RNA codon translation using Base-4 encoding
- Gene regulation via bitmask logic
- ABO blood group antigen compatibility

━━━━━━━━━━━━━━━━━
## Hardware Components
━━━━━━━━━━━━━━━━━

Component              | Role
-----------------------|---------------------------
STM32F103C8T6          | Main microcontroller
20×4 I2C LCD (0x27)    | Game display
8-Position DIP Switch  | Student input interface
TM1637 (×2)            | Score + countdown timer
Rotary Encoder         | Menu navigation
Active Buzzer          | Audio feedback
STM32 RTC              | Real-time clock on dashboard

━━━━━━━━━━━━━━━━━
## Pin Mapping
━━━━━━━━━━━━━━━━━

Switch S0–S7    → PA0–PA7   (Digital Input, Pull-up)
Encoder CLK     → PB13      (Interrupt)
Encoder DT      → PB12      (Input)
Encoder SW      → PB14      (Input, Pull-up)
Score TM1637    → PB10/PB11 (CLK/DIO)
Timer TM1637    → PB0/PB1   (CLK/DIO)
I2C LCD         → PB6/PB7   (SCL/SDA)
Buzzer          → PA15      (Digital Output)

━━━━━━━━━━━━━━━━━
## System Architecture
━━━━━━━━━━━━━━━━━

The firmware uses a UIState enum to manage 5 system 
states cleanly:

  STATE_DASHBOARD    — Live RTC clock display
  STATE_MENU         — Scrollable game selector
  STATE_GAME_RUNNING — Active game dispatcher
  STATE_RTC_SETUP    — Field-editable clock setup
  STATE_SELF_TEST    — Hardware diagnostic mode

Each game is assigned a unique activeGame ID and dispatched through a central loop() switch, keeping game logic fully modular and independently expandable.

━━━━━━━━━━━━━━━━━
## The "Ready Gate" Mechanic
━━━━━━━━━━━━━━━━━

Before timed games begin, the system requires the student to reset all 8 switches to the OFF (0) position. 
This physical zeroing serves two purposes:

1. Ensures a fair, clean starting state for every round
2. Acts as a deliberate mental reset — priming the 
   student before the timer begins

━━━━━━━━━━━━━━━━━
## Libraries Required
━━━━━━━━━━━━━━━━━

- Wire.h
- LiquidCrystal_I2C.h
- STM32RTC.h
- TM1637Display.h

━━━━━━━━━━━━━━━━━
## Classroom Deployment Notes
━━━━━━━━━━━━━━━━━

- All string literals use the F() macro to preserve RAM for game variables
- Games are fully independent — teachers can disable specific modules by commenting out menu entries
- Self Test mode allows hardware verification before class sessions
- RTC Setup mode enables accurate date/time stamping without a PC connection





> A hardware-based educational console that teaches Binary Logic, 
> Molecular Biology, Chemistry and Mathematics through 
> real-time, tactile switch-based gameplay.

---

## 📚 Subjects Covered
- 💻 Computer Science — Binary, Hex, Bitwise Logic
- 🧮 Mathematics — Prime Numbers, Binary Arithmetic  
- 📡 Communication — Morse Code
- ⚗️ Chemistry — Electron Shells, Periodic Table
- 🧬 Biology — Codon Translation, Gene Expression, Blood Match
