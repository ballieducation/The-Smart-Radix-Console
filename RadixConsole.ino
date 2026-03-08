#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <STM32RTC.h>
#include <TM1637Display.h>

// ================= PINS =================
#define CLK_LEFT   PB10
#define DIO_LEFT   PB11
#define CLK_RIGHT  PB0
#define DIO_RIGHT  PB1
#define ENC_A      PB13
#define ENC_B      PB12
#define ENC_SW     PB14
#define BUZZER     PA15

// ================= BINARY BLITZ VARS =================
int blitzTarget = 0;
int blitzScore = 0;
unsigned long blitzStartTime = 0;
const int blitzTimeLimit = 30; 

const int switchPins[8] = {PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7}; 


// ================= RADIX ROULETTE VARS =================
int rouletteTarget = 0;
int rouletteScore = 0;
uint8_t currentBase = 10; // 10 = Dec, 16 = Hex, 2 = Binary
unsigned long rouletteStartTime = 0;
const int rouletteTimeLimit = 20; // Faster than Blitz!


// ================= GLOBAL GAME VARIABLES =================
int activeGame = -1;       // Tracks which game is running
int scoreVal = 0;          // Shared score variable
int targetVal = 0;         // Shared target variable
unsigned long gameStartTime = 0; 

// --- BIT RUNNER SPECIFIC ---
//uint8_t runnerPos = 0;     // Current position of the bit
//int moveDelay = 800;       // Speed of movement
//unsigned
 //long lastMove = 0;
 uint8_t runnerPos = 0;      // Current position (0 to 7)
unsigned long lastMove = 0; // Timing tracker
int moveDelay = 800;        // Start speed (ms)


// --- FULL BINARY ARITHMETIC ---
int numA = 0;
int numB = 0;
char currentOp = '+';
int arithmeticTarget = 0;
uint8_t opType = 0; // 0=Add, 1=Sub, 2=Mul, 3=Div


// --- PRIME HUNTER V2 VARS ---
int hunterNumbers[8];
byte primeMask = 0;       // Each bit represents if the corresponding number is prime
bool rowCleared = false;



// Bitwise Game Variables
byte valA = 0;
byte valB = 0;
byte bitwiseTarget = 0;
char currentBitOpName[4] = "   ";

void lcdPrintBinary(byte val); // Just the name, no brackets/logic





// ================= OBJECTS =================
TM1637Display tmScore(CLK_LEFT, DIO_LEFT);
TM1637Display tmTimer(CLK_RIGHT, DIO_RIGHT);
LiquidCrystal_I2C lcd(0x27, 20, 4);
STM32RTC& rtc = STM32RTC::getInstance();

// ================= MENU DEFINITIONS =================
// Use const char* const to ensure these stay in Flash
const char* const mainMenu[] = {
  "Binary Blitz", "Radix Roulette", "Bit Runner", "Binary Arithmetic",
  "Bitwise Logic", "Prime Hunter", "Morse Trainer", "Electron Shells",
  "Periodic Table", "Codon Explorer", "Gene Expression", "Blood Match",
  "Self Test", "RTC Setup", "Return"
};
#define MENU_COUNT 15

enum UIState { STATE_DASHBOARD, STATE_MENU, STATE_RTC_SETUP, STATE_SELF_TEST, STATE_GAME_RUNNING };
UIState uiState = STATE_DASHBOARD;

// ================= GLOBAL VARIABLES =================
int menuIndex = 0;
int menuTopIndex = 0;
bool lastEncA;
unsigned long buttonPressStart = 0;
bool buttonHeld = false;

int selfCounter = 0;
bool selfLastA;
int editDay, editMonth, editYear, editHour, editMinute, editField;


// Morse Trainer Variables
char morseTarget;
String userMorseBuffer = "";
unsigned long pressStart = 0;
bool isKeyActive = false;

// Morse Reference Table
const char* morseTable[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", 
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", 
  "..-", "...-", ".--", "-..-", "-.--", "--.."
};

void nextMorseLetter() {
  morseTarget = 'A' + random(0, 26);
  userMorseBuffer = "";
}


// --- ELECTRON SHELLS & PERIODIC TABLE VARS ---
int targetAtomicNum = 0;
String elementName = "";
int requiredValence = 0;
int currentZ = 1;

// Element Names for Case 7 (L-Shell focus)
const char* const shellElements[] = {
  "Lithium", "Beryllium", "Boron", "Carbon", 
  "Nitrogen", "Oxygen", "Fluorine", "Neon"
};

// Valence electrons for all 118 elements (for Case 8)
const uint8_t valenceTable[118] = {
  1, 2,                                                             // H, He
  1, 2, 3, 4, 5, 6, 7, 8,                                           // Li to Ne
  1, 2, 3, 4, 5, 6, 7, 8,                                           // Na to Ar
  1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8,                   // K to Kr
  1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8,                   // Rb to Xe
  1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8,
  1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8 
};





// --- Global Declarations ---
// --- CODON EXPLORER GLOBALS ---
String targetAmino = "";
String targetCodon = "";

struct CodonData {
  char amino[12];
  char codon[4];
};

// Gamification: Focus on the "Vital" codons
const CodonData geneticLibrary[] = {
  {"START/MET", "AUG"}, {"STOP", "UAA"}, {"GLYCINE", "GGG"},
  {"VALINE", "GUU"}, {"ALANINE", "GCU"}, {"LYSINE", "AAA"},
  {"STOP/AMB", "UAG"}, {"CYSTEINE", "UGU"}
};



// --- GENE EXPRESSION GLOBALS ---
String targetPhenotype = "";
byte regulatoryMask = 0;

struct GeneData {
  char phenotype[16];
  byte mask; // The required switch pattern
};

const GeneData geneLibrary[] = {
  {"Blue Eyes",    0b10101010},
  {"Build Muscle", 0b11110000},
  {"Fast Metabolism", 0b00001111},
  {"Glow in Dark", 0b10011001},
  {"High Immunity", 0b01010101},
  {"Brain Power",  0b11001100}
};



// --- BLOOD MATCH GLOBALS ---
String patientType = "";
byte donorTargetMask = 0;

struct BloodData {
  char typeName[5];
  byte mask; // S0=A, S1=B, S2=Rh
};

const BloodData bloodLibrary[] = {
  {"A+",  0b101}, {"A-",  0b001},
  {"B+",  0b110}, {"B-",  0b010},
  {"AB+", 0b111}, {"AB-", 0b011},
  {"O+",  0b100}, {"O-",  0b000}
};










void nextElectronProblem() {
  int idx = random(0, 8); 
  elementName = shellElements[idx];
  requiredValence = idx + 1; 
  targetAtomicNum = idx + 3; // Li starts at Z=3
}

void nextElementProblem() {
  currentZ = random(1, 119);
}


void showElectronFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  lcd.setCursor(2, 1); lcd.print(F("SHELLS FINISHED!"));
  lcd.setCursor(4, 2); lcd.print(F("SCORE: ")); lcd.print(scoreVal);
  tmScore.showNumberDec(scoreVal);
  delay(5000); 
  uiState = STATE_MENU; showMenu();
}

void showPeriodicFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  lcd.setCursor(1, 1); lcd.print(F("PERIODIC OVER!"));
  lcd.setCursor(4, 2); lcd.print(F("SCORE: ")); lcd.print(scoreVal);
  tmScore.showNumberDec(scoreVal);
  delay(5000);
  uiState = STATE_MENU; showMenu();
}











// ================= HELPERS =================
void print2digit(int val) { 
  if (val < 10) lcd.print('0'); 
  lcd.print(val); 
}

// ================= DISPATCHER =================
void executeMenuAction(int index) {
  lcd.clear();
  switch(index) {
    case 0:  startBinaryBlitz(); break;
    case 1: startRadixRoulette(); break; // <-- NEW
    case 2: // Bit Runner
      uiState = STATE_GAME_RUNNING;
      activeGame = 2;
      scoreVal = 0;
      runnerPos = 0;
      moveDelay = 800; // Reset to slow speed
      gameStartTime = millis();
      lcd.clear();
      lcd.print(F("--- BIT RUNNER ---"));
      break;

   case 3: // Binary Arithmetic
  uiState = STATE_GAME_RUNNING;
  activeGame = 3;
  scoreVal = 0;
  gameStartTime = millis();
  nextBinaryArithmeticProblem(); // <--- CRITICAL: Generates first A and B
  lcd.clear();
  break;

case 4: // Bitwise Logic Entry
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("---BITWISE LOGIC---"));
  lcd.setCursor(0, 2);
  lcd.print(F("RESET ALL SWITCHES"));
  
  // Wait until all switches are toggled to 0
  while(readSwitches() != 0) {
    delay(50);
  }

  scoreVal = 0;
  nextBitwiseProblem();
  gameStartTime = millis(); // 60s Timer starts only after reset
  uiState = STATE_GAME_RUNNING;
  activeGame = 4; 
  lcd.clear();
  break;


case 5: // Prime Hunter Entry
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("---PRIME HUNTER---"));
  lcd.setCursor(0, 2);
  lcd.print(F("RESET ALL SWITCHES"));
  lcd.setCursor(0, 3);
  lcd.print(F("TO START CLOCK..."));

  // The "Ready" Gate
  while(readSwitches() != 0) {
    delay(50); 
  }

  // Initialize after reset
  scoreVal = 0;
  nextPrimeRow();
  gameStartTime = millis(); // Timer starts NOW
  uiState = STATE_GAME_RUNNING;
  activeGame = 5;
  lcd.clear();
  break;

case 6: // Morse Trainer Entry
  lcd.clear();
  lcd.print(F("MORSE TRAINER"));
  lcd.setCursor(0, 2);
  lcd.print(F("RESET ALL SWITCHES"));
  
  while(readSwitches() != 0) { delay(10); } // Reset Gate

  scoreVal = 0;
  nextMorseLetter();
  gameStartTime = millis();
  activeGame = 6;
  uiState = STATE_GAME_RUNNING;
  lcd.clear();
  break;



case 7: // Electron Shells Menu Selection
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("---ELECTRON SHELL---"));
  lcd.setCursor(0, 2);
  lcd.print(F("RESET ALL SWITCHES"));
  
  // Wait for safety reset
  while(readSwitches() != 0) { delay(10); }

  scoreVal = 0;
  nextElectronProblem();     // Pick first element
  gameStartTime = millis();  // Start timer
  activeGame = 7;            // Set ID to 7
  uiState = STATE_GAME_RUNNING;
  lcd.clear();
  break;









case 8: // Periodic Table Game Entry
  lcd.clear();
  lcd.print(F("PERIODIC TABLE"));
  lcd.setCursor(0, 2);
  lcd.print(F("RESET ALL SWITCHES"));
  
  while(readSwitches() != 0) { delay(10); } // Wait for 00000000

  scoreVal = 0;
  currentZ = random(1, 119); // Pick random Z from 1 to 118
  gameStartTime = millis();
  activeGame = 8;            // Assign unique ID
  uiState = STATE_GAME_RUNNING;
  lcd.clear();
  break;


case 9: // Codon Explorer
  lcd.clear();
  lcd.print(F("CODON EXPLORER"));
  while(readSwitches() != 0) { delay(10); } // Gamification: Clear the deck
  scoreVal = 0;
  nextCodonProblem();
  gameStartTime = millis();
  activeGame = 9;
  uiState = STATE_GAME_RUNNING;
  lcd.clear();
  break;


case 10: // Gene Expression Entry
  lcd.clear();
  lcd.print(F("EXPRESS:"));  // Static Label Row 0
  lcd.setCursor(0, 1);
  lcd.print(F(" GENOME:"));   // Static Label Row 1
  lcd.setCursor(0, 3);
  lcd.print(F("SEEKING HOMEOSTASIS")); // Static Label Row 3
  
  scoreVal = 0;
  nextGeneProblem();
  gameStartTime = millis();
  activeGame = 10;
  uiState = STATE_GAME_RUNNING;
  break;



case 11: // Blood Match Entry
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("PATIENT : "));
  lcd.setCursor(0, 1); lcd.print(F("DONOR   : "));
  lcd.setCursor(0, 3); lcd.print(F("S0=A S1=B S2=Rh")); // Tutorial Row
  
  scoreVal = 0;
  nextBloodProblem();
  gameStartTime = millis();
  activeGame = 11;
  uiState = STATE_GAME_RUNNING;
  break;





    case 12: startSelfTest();    break;
    case 13: uiState = STATE_RTC_SETUP; enterRTCSetup(); break;
    case 14: uiState = STATE_DASHBOARD; showDashboard(); break;
    default:
      lcd.setCursor(0,1); lcd.print(F("Starting..."));
      lcd.setCursor(0,2); lcd.print(mainMenu[index]);
      delay(1000);
      break;
  }
}

// ================= GAME LOGIC =================
void runBinaryBlitz() {
  int remaining = blitzTimeLimit - ((millis() - blitzStartTime) / 1000);
  if (remaining <= 0) {
    digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
    lcd.clear();
    lcd.setCursor(5, 1); lcd.print(F("GAME OVER!"));
    lcd.setCursor(4, 2); lcd.print(F("Score: ")); lcd.print(blitzScore);
    delay(3000);
    uiState = STATE_MENU; showMenu();
    return;
  }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(blitzScore);

  byte playerInput = 0;
  for (int i = 0; i < 8; i++) if (digitalRead(switchPins[i]) == LOW) playerInput |= (1 << i);

  lcd.setCursor(0, 1); lcd.print(F("TARGET: ")); lcd.print(blitzTarget); lcd.print(F("    "));
  lcd.setCursor(0, 2); lcd.print(F("YOURS : ")); lcd.print(playerInput); lcd.print(F("    "));

  if (playerInput == blitzTarget) {
    digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
    blitzScore += 10;
    blitzTarget = random(1, 255);
    blitzStartTime = millis();
    lcd.setCursor(0, 3); lcd.print(F("CORRECT! NEXT...   "));
    delay(500);
    lcd.setCursor(0, 3); lcd.print(F("                   "));
  }
}

void startBinaryBlitz() {
  uiState = STATE_GAME_RUNNING;
  blitzScore = 0;
  blitzTarget = random(1, 255);
  blitzStartTime = millis();
  lcd.clear();
  lcd.print(F("--- BINARY BLITZ ---"));
  tmScore.clear();
}

// ================= RADIX ROULETTE  =================
void startRadixRoulette() {
  uiState = STATE_GAME_RUNNING;
  rouletteScore = 0;
  rouletteTarget = random(1, 255);
  currentBase = 10; // Start with Decimal
  rouletteStartTime = millis();
  
  lcd.clear();
  lcd.print(F("--- RADIX ROULETTE ---"));
  tmScore.clear();
}

void runRadixRoulette() {
  int remaining = rouletteTimeLimit - ((millis() - rouletteStartTime) / 1000);

  if (remaining <= 0) {
    digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
    lcd.clear();
    lcd.setCursor(4, 1); lcd.print(F("ROULETTE OVER"));
    lcd.setCursor(4, 2); lcd.print(F("Score: ")); lcd.print(rouletteScore);
    delay(3000);
    uiState = STATE_MENU; showMenu();
    return;
  }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(rouletteScore);

  // Read switches
  byte playerInput = 0;
  for (int i = 0; i < 8; i++) if (digitalRead(switchPins[i]) == LOW) playerInput |= (1 << i);

  // Display Target based on current base
  lcd.setCursor(0, 1);
  if (currentBase == 10) {
    lcd.print(F("DEC TARGET: ")); lcd.print(rouletteTarget); 
  } else if (currentBase == 16) {
    lcd.print(F("HEX TARGET: 0x")); lcd.print(rouletteTarget, HEX);
  } else {
    lcd.print(F("BIN TARGET: ")); lcd.print(rouletteTarget, BIN);
  }
  lcd.print(F("    ")); // Clear trailing characters

  lcd.setCursor(0, 2);
  lcd.print(F("YOUR INPUT: ")); lcd.print(playerInput); lcd.print(F("    "));

  // Check Success
  if (playerInput == rouletteTarget) {
    digitalWrite(BUZZER, HIGH); delay(80); digitalWrite(BUZZER, LOW);
    rouletteScore += 15; // Higher reward for Radix
    rouletteTarget = random(1, 255);
    
    // Pick a new random base for the next round
    int r = random(0, 3);
    if (r == 0) currentBase = 10;
    else if (r == 1) currentBase = 16;
    else currentBase = 2;

    rouletteStartTime = millis(); // Reset timer
    lcd.setCursor(0, 3); lcd.print(F("GREAT! CHANGING... "));
    delay(600);
    lcd.setCursor(0, 3); lcd.print(F("                   "));
  }
}

// --- BIT RUNNER VARIABLES ---



byte readSwitches() {
  byte val = 0;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(switchPins[i]) == LOW) val |= (1 << i);
  }
  return val;
}
void runBitRunner() {
  // 1. Timer Logic
  int remaining = 30 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) {
    digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
    lcd.clear(); lcd.print(F("Runner Finished!"));
    lcd.setCursor(0, 2); lcd.print(F("Final Score: ")); lcd.print(scoreVal);
    delay(3000); uiState = STATE_MENU; showMenu(); return;
  }

  // 2. Display Updates
  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // 3. The Movement Logic
  if (millis() - lastMove > moveDelay) {
    lastMove = millis();
    runnerPos = (runnerPos + 1) % 8; // Move to next bit
    targetVal = (1 << runnerPos);    // Update the target bit value
    
    // Increase difficulty: speed up slightly every move
    if (moveDelay > 200) moveDelay -= 5; 
  }

  // 4. LCD Feedback
  lcd.setCursor(0, 1);
  lcd.print(F("CATCH BIT: "));
  // Show the bit position visually
  for(int i=7; i>=0; i--) {
    lcd.print(i == runnerPos ? '1' : '0');
  }

  lcd.setCursor(0, 2);
  byte input = readSwitches();
  lcd.print(F("YOUR POS:  "));
  for(int i=7; i>=0; i--) {
    lcd.print(bitRead(input, i) ? '1' : '0');
  }

  // 5. Scoring
  if (input == targetVal) {
    // We don't use a long delay here so the bit keeps moving
    scoreVal += 1; // Constant scoring for staying on the bit
    digitalWrite(BUZZER, (millis() % 200 < 50)); // Tiny blip when matched
  } else {
    digitalWrite(BUZZER, LOW);
  }
}


///////////////////////////////////
//////////////////////////////////////
////////////////////////////////////////
void lcdPrintBinary(byte val) {
  for (int i = 7; i >= 0; i--) {
    lcd.print(bitRead(val, i));
  }
}





void nextBinaryArithmeticProblem() {
  opType = random(0, 4); 

  switch (opType) {
    case 0: // ADDITION (A + B <= 255)
      numA = random(1, 150);
      numB = random(1, (255 - numA));
      currentOp = '+';
      arithmeticTarget = numA + numB;
      break;

    case 1: // SUBTRACTION (A >= B)
      numA = random(50, 255);
      numB = random(1, numA);
      currentOp = '-';
      arithmeticTarget = numA - numB;
      break;

    case 2: // MULTIPLICATION (Small operands)
      numA = random(2, 16); 
      numB = random(2, (255 / numA)); 
      currentOp = '*';
      arithmeticTarget = numA * numB;
      break;

    case 3: // DIVISION (Clean division)
      numB = random(2, 12); 
      arithmeticTarget = random(1, (255 / numB));
      numA = arithmeticTarget * numB; 
      currentOp = '/';
      break;
  }
}




void runBinaryArithmetic() {
  // 1. Timer Logic
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) {
    uiState = STATE_MENU; showMenu(); return;
  }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // 2. DRAW THE SCREEN (Do not use lcd.clear() here!)
  lcd.setCursor(0, 0);
  lcd.print(F("A: ")); lcdPrintBinary(numA);
  
  lcd.setCursor(0, 1);
  lcd.print(currentOp); lcd.print(F(": ")); lcdPrintBinary(numB);

  // 3. READ INPUT
  byte input = readSwitches();
  lcd.setCursor(0, 2);
  lcd.print(F("IN:")); lcdPrintBinary(input);
  lcd.print(F(" val:")); lcd.print(input);
  lcd.print(F("  ")); // Clear ghost numbers

  // 4. CHECK WIN
  if (input == arithmeticTarget) {
    digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
    scoreVal += 50;
    nextBinaryArithmeticProblem();
    lcd.clear(); // Clear once only when problem changes
  }
}



//===========================================================
//=============================================================
void nextPrimeRow() {
  primeMask = 0;
  for (int i = 0; i < 8; i++) {
    hunterNumbers[i] = random(2, 100); // Random numbers 2-99
    if (checkPrime(hunterNumbers[i])) {
      bitSet(primeMask, i); // Set the bit if it's prime
    }
  }
  // If a row has NO primes by luck, regenerate it
  if (primeMask == 0) nextPrimeRow(); 
}




void runPrimeHunter() {
  lcd.noCursor(); // Kills the blink on S0
  lcd.noBlink();  // Kills the blink on S0
  
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) {
    showPrimeFinalScore(); 
    return;
  }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // --- ROW 0: THE NUMBERS ---
  lcd.setCursor(0, 0); 
  for (int i = 7; i >= 0; i--) {
    if (hunterNumbers[i] < 10) lcd.print('0'); 
    lcd.print(hunterNumbers[i]);
    lcd.print(F(" ")); 
  }

  // --- ROW 1: P/C LABELS ---
  byte input = readSwitches();
  lcd.setCursor(0, 1);
  for (int i = 7; i >= 0; i--) {
    lcd.print(bitRead(input, i) ? F("P  ") : F("C  "));
  }

// --- ROW 3: STATUS MESSAGE (Row index 3, Column index 4) ---
  lcd.setCursor(4, 3); // 5th Column, 4th Row
  if (input == primeMask) {
    digitalWrite(BUZZER, HIGH); delay(200); digitalWrite(BUZZER, LOW);
    scoreVal += 80;
    
    // Clear the specific row 3 area and show match
    lcd.setCursor(0, 3); 
    lcd.print(F(" MATCH! RESET ALL   ")); 
    
    while(readSwitches() != 0) { delay(10); } // Wait for reset
    nextPrimeRow();
    lcd.clear(); // Clear all to prevent overlap on next round
  } else {
    // Print starting at the 5th column
    lcd.print(F("MAP THE PRIMES   ")); // Spaces at end clear any old "MATCH" text
  }




}



void showPrimeFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(1000); digitalWrite(BUZZER, LOW);
  
  // Clean Final Display
  lcd.setCursor(0, 1);
  lcd.print(F("    TIME EXPIRED    "));
  lcd.setCursor(0, 2);
  lcd.print(F("   FINAL SCORE:"));
  lcd.print(scoreVal);
  
  delay(5000); // 5 seconds to read
  uiState = STATE_MENU;
  showMenu();
}



bool checkPrime(int n) {
  if (n <= 1) return false;
  for (int i = 2; i * i <= n; i++) {
    if (n % i == 0) return false;
  }
  return true;
}

//===========================================================
//===========================================================
// Global variables for Bitwise Logic
void nextBitwiseProblem() {
  valA = random(0, 255);
  valB = random(0, 255);
  int op = random(0, 7); // Increased range for 7 operators

  switch (op) {
    case 0: 
      strcpy(currentBitOpName, "AND"); 
      bitwiseTarget = valA & valB; 
      break;
    case 1: 
      strcpy(currentBitOpName, "OR "); 
      bitwiseTarget = valA | valB; 
      break;
    case 2: 
      strcpy(currentBitOpName, "XOR"); 
      bitwiseTarget = valA ^ valB; 
      break;
    case 3: 
      strcpy(currentBitOpName, "NAN"); // NAND
      bitwiseTarget = ~(valA & valB); 
      break;
    case 4: 
      strcpy(currentBitOpName, "NOR"); // NOR
      bitwiseTarget = ~(valA | valB); 
      break;
    case 5: 
      strcpy(currentBitOpName, "XNO"); // XNOR
      bitwiseTarget = ~(valA ^ valB); 
      break;
    case 6: 
      strcpy(currentBitOpName, "NOT"); // NOT (A only)
      bitwiseTarget = ~valA; 
      break;
  }
}


void nextBitwiseProblem1() {
  valA = random(0, 255);
  valB = random(0, 255);
  int op = random(0, 3); // 0=AND, 1=OR, 2=XOR

  if (op == 0) { 
    strcpy(currentBitOpName, "AND"); 
    bitwiseTarget = valA & valB; 
  } else if (op == 1) { 
    strcpy(currentBitOpName, "OR "); 
    bitwiseTarget = valA | valB; 
  } else { 
    strcpy(currentBitOpName, "XOR"); 
    bitwiseTarget = valA ^ valB; 
  }
}




void showBitwiseFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  
  // Show final score on both LCD and TM1637
  lcd.setCursor(2, 1);
  lcd.print(F("LOGIC TIME OVER!"));
  lcd.setCursor(4, 2);
  lcd.print(F("SCORE: "));
  lcd.print(scoreVal);
  
  tmScore.showNumberDec(scoreVal);
  
  delay(5000); // 5 seconds to view result
  uiState = STATE_MENU;
  showMenu();
}

void runBitwiseGame() {
  lcd.noCursor(); 
  lcd.noBlink();

  // 1. Timer & TM1637 Update
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) {
    showBitwiseFinalScore(); 
    return;
  }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // 2. Aligned LCD Layout (Starts at Column 5 for vertical stacking)
  // Row 0: Variable A
  lcd.setCursor(0, 0);
  lcd.print(F("A:   ")); 
  lcd.setCursor(4, 0); // Jump to 5th column
  lcdPrintBinary(valA); 

  // Row 1: Operator and Variable B
  lcd.setCursor(0, 1);
  lcd.print(currentBitOpName); 
  lcd.print(F(":  ")); 
  lcd.setCursor(4, 1); // Jump to 5th column
  lcdPrintBinary(valB);

  // Row 2: Live Switch Input
  byte input = readSwitches();
  lcd.setCursor(0, 2);
  lcd.print(F("I:   ")); 
  lcd.setCursor(4, 2); // Jump to 5th column
  lcdPrintBinary(input);

  // 3. Match Logic & Reset Gate
  lcd.setCursor(0, 3);
  if (input == bitwiseTarget) {
    digitalWrite(BUZZER, HIGH); delay(200); digitalWrite(BUZZER, LOW);
    scoreVal += 100;
    
    // Using padding to clear "SOLVE LOGIC"
    lcd.print(F(" MATCH! RESET SW... "));
    
    // Wait for physical reset (All switches to 0)
    while(readSwitches() != 0) { 
      delay(10); 
    }
    
    nextBitwiseProblem();
    lcd.clear(); // Clear display for next problem
  } else {
    // Aligned status message
    lcd.setCursor(4, 3);
    lcd.print(F("SOLVE THE LOGIC "));
  }
}


//===========================================================
void showMorseFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  
  lcd.setCursor(2, 1);
  lcd.print(F("MORSE TIME OVER!"));
  lcd.setCursor(4, 2);
  lcd.print(F("SCORE: "));
  lcd.print(scoreVal);
  
  tmScore.showNumberDec(scoreVal);
  
  delay(5000); 
  uiState = STATE_MENU;
  showMenu();
}




void runMorseTrainer() {
  lcd.noCursor();
  
  // 1. Timer Logic
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) {
    showMorseFinalScore(); 
    return;
  }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // 2. Display Layout
  lcd.setCursor(0, 0);
  lcd.print(F("TARGET: ")); lcd.print(morseTarget);
  lcd.setCursor(0, 1);
  lcd.print(F("GOAL  : ")); lcd.print(morseTable[morseTarget - 'A']);
  lcd.setCursor(0, 2);
  lcd.print(F("INPUT : ")); lcd.print(userMorseBuffer);
  lcd.print(F("    ")); // Clear trailing characters

  // 3. Telegraph Key Logic (S0)
  bool s0State = bitRead(readSwitches(), 0); 

  if (s0State) {
    if (!isKeyActive) {
      pressStart = millis();
      isKeyActive = true;
      digitalWrite(BUZZER, HIGH);
    }
    
    // LIVE PROGRESS BAR on Row 3
    unsigned long currentHold = millis() - pressStart;
    lcd.setCursor(0, 3);
    lcd.print(F("["));
    int bars = map(constrain(currentHold, 0, 300), 0, 300, 0, 10);
    for(int i=0; i<10; i++) {
      if (i < bars) lcd.print('='); else lcd.print(' ');
    }
    lcd.print(currentHold < 250 ? F("] DIT") : F("] DAH"));
  } 
  else if (!s0State && isKeyActive) {
    unsigned long duration = millis() - pressStart;
    isKeyActive = false;
    digitalWrite(BUZZER, LOW);
    lcd.setCursor(0, 3); lcd.print(F("                    ")); // Clear bar

    if (duration < 250) userMorseBuffer += ".";
    else userMorseBuffer += "-";
  }

  // 4. Match & Failure Logic
  if (userMorseBuffer == morseTable[morseTarget - 'A']) {
    scoreVal += 50;
    lcd.setCursor(0, 3);
    lcd.print(F("   CORRECT! +50     "));
    delay(800);
    nextMorseLetter();
    lcd.clear();
  } 
  // ERROR CHECK: If input length is wrong or characters don't match
  else if (userMorseBuffer.length() >= 5) { 
       lcd.setCursor(0, 3);
       lcd.print(F("   LIMIT! RESET     "));
       delay(800);
       userMorseBuffer = ""; 
       lcd.clear();
  }
}
//=============================================================
struct Element {
  char name[12];
  int atomicNum;
  int valence; // How many switches need to be ON
};

// Expanded list including different periods
const Element elements[] = {
  {"Hydrogen", 1, 1}, {"Helium", 2, 2},     // Period 1 (K-shell)
  {"Lithium", 3, 1},  {"Carbon", 6, 4},     // Period 2 (L-shell)
  {"Neon", 10, 8},    {"Sodium", 11, 1},    // Period 3 (M-shell)
  {"Silicon", 14, 4}, {"Argon", 18, 8},
  {"Potassium", 19, 1}, {"Calcium", 20, 2}  // Period 4
};


void runElectronShells() {
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) { showElectronFinalScore(); return; }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  lcd.setCursor(0, 0);
  lcd.print(elementName); 
  lcd.print(F(" (Z=")); lcd.print(targetAtomicNum); lcd.print(F(")"));

  byte input = readSwitches();
  int currentValence = 0;
  for(int i=0; i<8; i++) if(bitRead(input, i)) currentValence++;

  lcd.setCursor(0, 1);
  lcd.print(F("CONFIG: 2, ")); lcd.print(currentValence); 
  
  lcd.setCursor(0, 2);
  lcd.print(F("VALENCE: "));
  for(int i=0; i<8; i++) lcd.print(i < currentValence ? 'o' : '.');

  if (currentValence == requiredValence) {
    lcd.setCursor(0, 3);
    lcd.print(F(">> ATOM STABLE! <<  "));
    digitalWrite(BUZZER, HIGH); delay(200); digitalWrite(BUZZER, LOW);
    scoreVal += 100;
    nextElectronProblem();
    lcd.clear();
  } else {
    lcd.setCursor(0, 3);
    lcd.print(F("IONIC STATE...      "));
  }
}


//===========================================================
struct ElementData {
  char symbol[3];   // e.g., "He", "Li", "U"
  uint8_t valence;  // Electrons in the outermost shell (1-8)
};

// Storing in PROGMEM saves RAM for your other games
const ElementData periodicTable[] = {
  {"H", 1}, {"He", 2},                                              // Period 1
  {"Li", 1}, {"Be", 2}, {"B", 3}, {"C", 4}, {"N", 5}, {"O", 6}, {"F", 7}, {"Ne", 8}, // Period 2
  {"Na", 1}, {"Mg", 2}, {"Al", 3}, {"Si", 4}, {"P", 5}, {"S", 6}, {"Cl", 7}, {"Ar", 8}, // Period 3
  // ... and so on for all 118.
};


void runPeriodicTableGame() {
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) { showPeriodicFinalScore(); return; }

  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // --- ROW 0: ELEMENT HEADER ---
  lcd.setCursor(0, 0);
  lcd.print(F("ELEMENT: [ "));
  // Assuming you have symbols; if not, just show Z
  lcd.print(currentZ); 
  lcd.print(F(" ]    ")); 

  // --- ROW 1: THE CHALLENGE ---
  int targetValence = valenceTable[currentZ - 1];
  lcd.setCursor(0, 1);
  lcd.print(F("NEED: ")); lcd.print(targetValence);
  lcd.print(F(" VALENCE e-"));

  // --- ROW 2: LIVE SWITCH VISUALIZER ---
  byte input = readSwitches();
  int userCount = 0;
  for(int i=0; i<8; i++) if(bitRead(input, i)) userCount++;

  lcd.setCursor(0, 2);
  lcd.print(F("SHELL: "));
  for(int i=0; i<8; i++) {
    // Show 'o' for electron, '.' for empty slot
    lcd.print(i < userCount ? 'o' : '.'); 
  }
  lcd.print(F(" (")); lcd.print(userCount); lcd.print(F(")"));

  // --- ROW 3: STATUS & MATCH ---
  lcd.setCursor(0, 3);
  if (userCount == targetValence) {
    // Only grant points if bits are filled from right-to-left (standard configuration)
    byte correctBits = (1 << targetValence) - 1;
    if (input == correctBits) {
      lcd.print(F(">> STABLE ATOM! <<  "));
      digitalWrite(BUZZER, HIGH); delay(150); digitalWrite(BUZZER, LOW);
      scoreVal += 100;
      nextElementProblem();
      lcd.clear();
    } else {
      lcd.print(F("ALIGN FROM RIGHT S0 "));
    }
  } else {
    lcd.print(F("IONIZING...         "));
  }
}


//===========================================================
void showCodonFinalScore() {
  lcd.clear();
  lcd.setCursor(2, 1); lcd.print(F("GENOME COMPLETE!"));
  lcd.setCursor(4, 2); lcd.print(F("SCORE: ")); lcd.print(scoreVal);
  tmScore.showNumberDec(scoreVal);
  delay(5000);
  uiState = STATE_MENU; showMenu();
}
void nextCodonProblem() {
  int r = random(0, 6);
  targetAmino = geneticLibrary[r].amino;
  targetCodon = geneticLibrary[r].codon;
}

void runCodonExplorer() {
  // 1. Gamified Timer
  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) { showCodonFinalScore(); return; }
  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // 2. UI - The "Lab Analysis" Screen
  lcd.setCursor(0, 0);
  lcd.print(F("AMINO: ")); lcd.print(targetAmino);

  // 3. Decoding Logic (Teaching Base-4)
  byte input = readSwitches();
  String userSequence = "";
  const char* baseMap = "ACGU"; 

  // Map pairs: S0-S1 (Base 1), S2-S3 (Base 2), S4-S5 (Base 3)
  for (int i = 0; i < 6; i += 2) {
    int bit0 = bitRead(input, i);
    int bit1 = bitRead(input, i + 1);
    int baseIndex = (bit1 << 1) | bit0; // Binary to Dec (0-3)
    userSequence += baseMap[baseIndex];
  }

  lcd.setCursor(0, 1);
  lcd.print(F("CODON: ")); lcd.print(userSequence);
  
  // 4. Instructional Legend (The "Cheat Sheet")
  lcd.setCursor(0, 2);
  lcd.print(F("0=A 1=C 2=G 3=U"));

  // 5. Win Logic
  lcd.setCursor(0, 3);
  if (userSequence == targetCodon) {
    lcd.print(F(">> PROTEIN BUILT! <<"));
    digitalWrite(BUZZER, HIGH); delay(150); digitalWrite(BUZZER, LOW);
    scoreVal += 150; // High reward for complex logic
    nextCodonProblem();
    lcd.clear();
  } else {
    lcd.print(F("WAITING FOR TRNA... "));
  }
}
//========================runGeneExpression=====================================
String scrollText(String text, int width, int delayTime) {
  if (text.length() <= width) {
    String padded = text;
    while(padded.length() < width) padded += " ";
    return padded;
  }
  // Add a gap so the start and end of the string don't touch
  String extendedText = text + " --- "; 
  int offset = (millis() / delayTime) % extendedText.length();
  
  String result = "";
  for (int i = 0; i < width; i++) {
    result += extendedText[(offset + i) % extendedText.length()];
  }
  return result;
}


void nextGeneProblem() {
  int r = random(0, 6);
  targetPhenotype = geneLibrary[r].phenotype;
  regulatoryMask = geneLibrary[r].mask;
  // NO lcd.clear() here!
}

void showGeneFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  lcd.setCursor(2, 1); lcd.print(F("EVOLUTION PAUSED"));
  lcd.setCursor(4, 2); lcd.print(F("SCORE: ")); lcd.print(scoreVal);
  tmScore.showNumberDec(scoreVal);
  delay(5000);
  uiState = STATE_MENU; showMenu();
}

void runGeneExpression() {
  // --- REFRESH THROTTLE ---
  static unsigned long lastLCDUpdate = 0;
  if (millis() - lastLCDUpdate < 150) return; 
  lastLCDUpdate = millis();

  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) { showGeneFinalScore(); return; }
  
  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // --- ROW 0: DATA ONLY (Starts at index 9) ---
  lcd.setCursor(9, 0); 
  lcd.print(scrollText(targetPhenotype, 11, 350)); 

  // --- ROW 1: BITS ONLY (Starts at index 9) ---
  byte input = readSwitches();
  lcd.setCursor(9, 1); 
  for(int i=7; i>=0; i--) {
    lcd.print(bitRead(input, i) ? '1' : '0');
  }

  // --- ROW 2: FEEDBACK (This row can be dynamic) ---
  lcd.setCursor(0, 2);
  int diff = 0;
  for(int i=0; i<8; i++) if(bitRead(input, i) != bitRead(regulatoryMask, i)) diff++;
  
  if (input == regulatoryMask) {
    lcd.print(F("MATCH: >> STABLE << "));
  } else {
    lcd.print(F("ERRORS: ")); lcd.print(diff); 
    lcd.print(F(" BITS    ")); // Spaces wipe old numbers
  }

  // --- ROW 3: SUCCESS CHECK ---
  if (input == regulatoryMask) {
    digitalWrite(BUZZER, HIGH); delay(150); digitalWrite(BUZZER, LOW);
    scoreVal += 100;
    nextGeneProblem(); // Logic update only
    // No lcd.clear() here!
  }
}


//=============================================================
void nextBloodProblem() {
  int r = random(0, 8);
  patientType = bloodLibrary[r].typeName;
  donorTargetMask = bloodLibrary[r].mask;
}

void showBloodFinalScore() {
  lcd.clear();
  digitalWrite(BUZZER, HIGH); delay(500); digitalWrite(BUZZER, LOW);
  lcd.setCursor(2, 1); lcd.print(F("MATCHING ENDED"));
  lcd.setCursor(4, 2); lcd.print(F("SCORE: ")); lcd.print(scoreVal);
  tmScore.showNumberDec(scoreVal);
  delay(5000);
  uiState = STATE_MENU; showMenu();
}

void runBloodMatch() {
  static unsigned long lastLCDUpdate = 0;
  if (millis() - lastLCDUpdate < 150) return; 
  lastLCDUpdate = millis();

  int remaining = 60 - ((millis() - gameStartTime) / 1000);
  if (remaining <= 0) { showBloodFinalScore(); return; }
  
  tmTimer.showNumberDec(remaining);
  tmScore.showNumberDec(scoreVal);

  // Row 0: The Patient Target
  lcd.setCursor(10, 0); 
  lcd.print(patientType); lcd.print(F("    "));

  // Row 1: The Donor (Physical Switch Input)
  byte input = readSwitches() & 0x07; // Only bits 0, 1, 2
  lcd.setCursor(10, 1);
  lcd.print(bitRead(input, 0) ? 'A' : '-');
  lcd.print(bitRead(input, 1) ? 'B' : '-');
  lcd.print(bitRead(input, 2) ? '+' : '-');
  lcd.print(F("    "));

  // Row 2: Live Compatibility Status
  lcd.setCursor(0, 2);
  if (input == donorTargetMask) {
    lcd.print(F("STATUS: >> SAFE <<  "));
  } else {
    lcd.print(F("STATUS: !!DANGER!!  "));
  }

  // Row 3: Success Logic
  if (input == donorTargetMask) {
    digitalWrite(BUZZER, HIGH); delay(150); digitalWrite(BUZZER, LOW);
    scoreVal += 100;
    nextBloodProblem();
    // No lcd.clear() to prevent label blinking
  }
}


//=============================================================
// ================= CORE =================
void setup() {
  pinMode(ENC_A, INPUT_PULLUP); pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP); pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  for(int i=0; i<8; i++) pinMode(switchPins[i], INPUT_PULLUP);

  //rtc.setClockSource(STM32RTC::LSI_CLOCK);    (LSI — no crystal, no battery backup)
  rtc.setClockSource(STM32RTC::LSE_CLOCK);  //(LSE — uses crystal + CR2032 backup)
  rtc.begin(); 
  if (!rtc.isTimeSet()) 
  { 
    rtc.setTime(12, 0, 0); 
    rtc.setDate(02, 3, 26); 
    }

  tmScore.setBrightness(0x0f); tmTimer.setBrightness(0x0f);
  lcd.init(); lcd.backlight();
  
  lastEncA = digitalRead(ENC_A);
  showDashboard();
}

void loop() {
  handleEncoder();
  handleButton();
  switch (uiState) {
    case STATE_DASHBOARD:    updateDashboardTick(); break;
    case STATE_SELF_TEST:    runSelfTest(); break;
    //case STATE_GAME_RUNNING: runBinaryBlitz(); break;
    case STATE_GAME_RUNNING: 
       if (menuIndex == 0) runBinaryBlitz();
       else if (menuIndex == 1) runRadixRoulette();
       else if (activeGame == 2) runBitRunner(); // <--- Add this
       else if (activeGame == 3) runBinaryArithmetic(); // <--- CALL HERE 
       else if (activeGame == 4) runBitwiseGame();       // <--- ADD THIS CALL FOR CASE 4
       else if (activeGame == 5) runPrimeHunter(); // <--- CALL HERE 
       else if (activeGame == 6) runMorseTrainer(); // <--- CALL HERE 
       else if (activeGame == 7) runElectronShells();
       else if (activeGame == 8) runPeriodicTableGame(); // <--- NEW CALL
       else if (activeGame == 9) runCodonExplorer();
       else if (activeGame == 10) runGeneExpression(); // Logic Gates in Biology
       else if (activeGame == 11) runBloodMatch();  // Logic Gates in Biology
       break;

  }
}

// ================= UI HANDLERS =================
void handleEncoder() {
  bool currentA = digitalRead(ENC_A);
  if (currentA == LOW && lastEncA == HIGH) {
    int dir = (digitalRead(ENC_B) != currentA) ? 1 : -1;
    if (uiState == STATE_MENU) {
      menuIndex = (menuIndex + dir + MENU_COUNT) % MENU_COUNT;
      showMenu();
    } else if (uiState == STATE_RTC_SETUP) adjustRTCField(dir);
  }
  lastEncA = currentA;
}

void handleButton() {
  bool btn = digitalRead(ENC_SW);
  if (btn == LOW && !buttonHeld) { buttonPressStart = millis(); buttonHeld = true; }
  if (btn == HIGH && buttonHeld) {
    unsigned long duration = millis() - buttonPressStart;
    buttonHeld = false;
    if (duration < 600) {
      if (uiState == STATE_DASHBOARD) { uiState = STATE_MENU; showMenu(); } 
      else if (uiState == STATE_MENU) executeMenuAction(menuIndex);
      else if (uiState == STATE_RTC_SETUP) { editField = (editField + 1) % 5; positionCursor(); }
      else if (uiState == STATE_SELF_TEST) { uiState = STATE_MENU; showMenu(); }
    } else {
      if (uiState == STATE_RTC_SETUP) saveRTC();
      else { uiState = STATE_DASHBOARD; showDashboard(); }
    }
  }
}

void showMenu() {
  lcd.clear(); lcd.print(F("--- RADIX MENU ---"));
  if (menuIndex < menuTopIndex) menuTopIndex = menuIndex;
  if (menuIndex > menuTopIndex + 2) menuTopIndex = menuIndex - 2;
  for (int i = 0; i < 3; i++) {
    int itemIdx = menuTopIndex + i;
    if (itemIdx >= MENU_COUNT) break;
    lcd.setCursor(0, i + 1);
    lcd.print(itemIdx == menuIndex ? F("> ") : F("  "));
    lcd.print(mainMenu[itemIdx]);
  }
}

void updateDashboardTick() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    lcd.setCursor(0,1);
    int y = rtc.getYear();
    if (y < 100) y += 2000;
    print2digit(rtc.getDay()); lcd.print('/');
    print2digit(rtc.getMonth()); lcd.print('/');
    lcd.print(y); lcd.print(F("  "));
    print2digit(rtc.getHours()); lcd.print(':');
    print2digit(rtc.getMinutes()); lcd.print(':');
    print2digit(rtc.getSeconds());
  }
}

void showDashboard() {
  lcd.clear(); lcd.print(F("RADIX MASTER v1.1"));
  lcd.setCursor(0,3); lcd.print(F("Press to Explore"));
}

void startSelfTest() {
  uiState = STATE_SELF_TEST;
  lcd.clear(); lcd.print(F("SELF TEST MODE"));
  selfCounter = 0; tmScore.clear(); tmTimer.clear();
}

void runSelfTest() {
  byte binVal = 0;
  for (int i = 0; i < 8; i++) if (digitalRead(switchPins[i]) == LOW) binVal |= (1 << i);
  bool curA = digitalRead(ENC_A);
  if (curA != selfLastA && curA == LOW) {
    if (digitalRead(ENC_B) != curA) selfCounter++; else selfCounter--;
    digitalWrite(BUZZER, HIGH); delay(10); digitalWrite(BUZZER, LOW);
  }
  selfLastA = curA;
  tmScore.showNumberDec(binVal);
  tmTimer.showNumberDec(selfCounter);
  lcd.setCursor(0,1); lcd.print(F("BIN: "));
  for(int i=7; i>=0; i--) lcd.print(bitRead(binVal, i));
  lcd.setCursor(0,2); lcd.print(F("DEC:")); lcd.print(binVal); 
  lcd.print(F(" HEX:")); lcd.print(binVal, HEX); lcd.print(F("   "));
  lcd.setCursor(0,3);
  if (digitalRead(ENC_SW) == LOW) { lcd.print(F("BTN: PRESSED   ")); digitalWrite(BUZZER, HIGH); } 
  else { lcd.print(F("BTN: OPEN      ")); digitalWrite(BUZZER, LOW); }
}

void enterRTCSetup() {
  editDay = rtc.getDay(); editMonth = rtc.getMonth(); 
  editYear = rtc.getYear() + 2000;
  editHour = rtc.getHours(); editMinute = rtc.getMinutes();
  editField = 0; lcd.cursor(); lcd.blink(); showRTCSetup();
}

void showRTCSetup() {
  lcd.setCursor(0,1); lcd.print(F("D:")); print2digit(editDay);
  lcd.print(F(" M:")); print2digit(editMonth);
  lcd.print(F(" Y:")); lcd.print(editYear);
  lcd.setCursor(0,2); lcd.print(F("Time: ")); print2digit(editHour);
  lcd.print(':'); print2digit(editMinute);
  positionCursor();
}

void adjustRTCField(int delta) {
  if(editField==0) editDay = constrain(editDay+delta,1,31);
  else if(editField==1) editMonth = constrain(editMonth+delta,1,12);
  else if(editField==2) editYear = constrain(editYear+delta,2024,2099);
  else if(editField==3) editHour = constrain(editHour+delta,0,23);
  else if(editField==4) editMinute = constrain(editMinute+delta,0,59);
  showRTCSetup();
}

void positionCursor() {
  if(editField==0) lcd.setCursor(2,1);
  else if(editField==1) lcd.setCursor(7,1);
  else if(editField==2) lcd.setCursor(12,1);
  else if(editField==3) lcd.setCursor(6,2);
  else if(editField==4) lcd.setCursor(9,2);
}

void saveRTC() {
  rtc.setDate(editDay, editMonth, editYear - 2000);
  rtc.setTime(editHour, editMinute, 0);
  lcd.noCursor(); lcd.noBlink();
  uiState = STATE_DASHBOARD; showDashboard();

}



