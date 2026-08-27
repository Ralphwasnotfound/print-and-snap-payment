#include <Servo.h>

// =====================================================
// SNAP AND PRINT
// COIN DISPENSER
// =====================================================
//
// ₱1  -> D8 / A1
// ₱5  -> D9 / A2
// ₱10 -> D10 / A3
// ₱20 -> D11 / A4
//
// IR SENSORS ARE MONITORING ONLY.
//
// The IR sensors DO NOT control the servos.
// The servos operate using timing.
//
// =====================================================


// =====================================================
// SERVOS
// =====================================================

Servo servo1;
Servo servo5;
Servo servo10;
Servo servo20;


// =====================================================
// SERVO PINS
// =====================================================

const byte SERVO_1_PIN  = 8;
const byte SERVO_5_PIN  = 9;
const byte SERVO_10_PIN = 10;
const byte SERVO_20_PIN = 11;


// =====================================================
// IR SENSOR PINS
// =====================================================

const byte IR_1_PIN  = A1;
const byte IR_5_PIN  = A2;
const byte IR_10_PIN = A3;
const byte IR_20_PIN = A4;


// =====================================================
// SERVO ANGLES
// =====================================================

const byte HOME_ANGLE = 0;

const byte PUSH_1_ANGLE  = 30;
const byte PUSH_5_ANGLE  = 40;
const byte PUSH_10_ANGLE = 40;
const byte PUSH_20_ANGLE = 40;


// =====================================================
// SERVO TIMING
// =====================================================

// Time servo stays at PUSH position
const unsigned long COIN_PUSH_HOLD = 500;

// Time between dispensing cycles
const unsigned long COIN_REPEAT_DELAY = 500;


// =====================================================
// IR MONITORING
// =====================================================

// Small debounce so one coin doesn't accidentally
// create multiple detections.
const unsigned long IR_DEBOUNCE = 100;


// =====================================================
// MONITOR COUNTERS
// =====================================================

volatile unsigned int detected1 = 0;
volatile unsigned int detected5 = 0;
volatile unsigned int detected10 = 0;
volatile unsigned int detected20 = 0;


// =====================================================
// INITIAL IR STATES
// =====================================================

byte lastIR1State;
byte lastIR5State;
byte lastIR10State;
byte lastIR20State;

unsigned long lastIR1Time = 0;
unsigned long lastIR5Time = 0;
unsigned long lastIR10Time = 0;
unsigned long lastIR20Time = 0;


// =====================================================
// INITIALIZE COIN DISPENSER
// =====================================================

void setupCoinDispenser()
{
  // ===================================================
  // ATTACH SERVOS
  // ===================================================

  servo1.attach(SERVO_1_PIN);
  servo5.attach(SERVO_5_PIN);
  servo10.attach(SERVO_10_PIN);
  servo20.attach(SERVO_20_PIN);


  // ===================================================
  // IR SENSORS
  // ===================================================

  pinMode(IR_1_PIN, INPUT);
  pinMode(IR_5_PIN, INPUT);
  pinMode(IR_10_PIN, INPUT);
  pinMode(IR_20_PIN, INPUT);


  // ===================================================
  // READ INITIAL SENSOR STATES
  // ===================================================

  lastIR1State = digitalRead(IR_1_PIN);
  lastIR5State = digitalRead(IR_5_PIN);
  lastIR10State = digitalRead(IR_10_PIN);
  lastIR20State = digitalRead(IR_20_PIN);


  // ===================================================
  // DEFAULT POSITION = HOME
  // ===================================================

  coinDispensersHome();

  delay(500);


  Serial.println(F("COIN_DISPENSER:READY"));
}


// =====================================================
// IR MONITOR
// =====================================================
//
// The IR sensors ONLY count coins.
//
// LOW = coin detected
//
// They do NOT stop or control the servo.
//
// =====================================================

void updateCoinSensors()
{
  unsigned long now = millis();

  // ===================================================
  // ₱1 SENSOR
  // ===================================================

  byte state1 = digitalRead(IR_1_PIN);

  if (state1 != lastIR1State)
  {
    lastIR1State = state1;

    // Detect falling edge
    if (state1 == LOW)
    {
      if (now - lastIR1Time >= IR_DEBOUNCE)
      {
        detected1++;

        lastIR1Time = now;

        Serial.print(F("IR:PHP1 COIN DETECTED #"));
        Serial.println(detected1);
      }
    }
  }


  // ===================================================
  // ₱5 SENSOR
  // ===================================================

  byte state5 = digitalRead(IR_5_PIN);

  if (state5 != lastIR5State)
  {
    lastIR5State = state5;

    if (state5 == LOW)
    {
      if (now - lastIR5Time >= IR_DEBOUNCE)
      {
        detected5++;

        lastIR5Time = now;

        Serial.print(F("IR:PHP5 COIN DETECTED #"));
        Serial.println(detected5);
      }
    }
  }


  // ===================================================
  // ₱10 SENSOR
  // ===================================================

  byte state10 = digitalRead(IR_10_PIN);

  if (state10 != lastIR10State)
  {
    lastIR10State = state10;

    if (state10 == LOW)
    {
      if (now - lastIR10Time >= IR_DEBOUNCE)
      {
        detected10++;

        lastIR10Time = now;

        Serial.print(F("IR:PHP10 COIN DETECTED #"));
        Serial.println(detected10);
      }
    }
  }


  // ===================================================
  // ₱20 SENSOR
  // ===================================================

  byte state20 = digitalRead(IR_20_PIN);

  if (state20 != lastIR20State)
  {
    lastIR20State = state20;

    if (state20 == LOW)
    {
      if (now - lastIR20Time >= IR_DEBOUNCE)
      {
        detected20++;

        lastIR20Time = now;

        Serial.print(F("IR:PHP20 COIN DETECTED #"));
        Serial.println(detected20);
      }
    }
  }
}


// =====================================================
// RESET MONITOR COUNTERS
// =====================================================

void resetCoinMonitoring()
{
  detected1 = 0;
  detected5 = 0;
  detected10 = 0;
  detected20 = 0;

  Serial.println(F("IR MONITOR COUNTERS RESET"));
}


// =====================================================
// REPORT ₱1 RESULT
// =====================================================

void reportOnePesoResult(int expected)
{
  Serial.println(F("--------------------------------------"));
  Serial.println(F("PHP 1 DISPENSER LOG"));

  Serial.print(F("EXPECTED: "));
  Serial.println(expected);

  Serial.print(F("IR DETECTED: "));
  Serial.println(detected1);

  if (detected1 == expected)
  {
    Serial.println(F("RESULT: OK"));
  }
  else
  {
    Serial.println(F("RESULT: FAULT"));
  }

  Serial.println(F("--------------------------------------"));
}


// =====================================================
// REPORT ₱5 RESULT
// =====================================================

void reportFivePesoResult()
{
  Serial.println(F("--------------------------------------"));
  Serial.println(F("PHP 5 DISPENSER LOG"));

  Serial.println(F("EXPECTED: 1"));

  Serial.print(F("IR DETECTED: "));
  Serial.println(detected5);

  if (detected5 == 1)
  {
    Serial.println(F("RESULT: OK"));
  }
  else
  {
    Serial.println(F("RESULT: FAULT"));
  }

  Serial.println(F("--------------------------------------"));
}


// =====================================================
// REPORT ₱10 RESULT
// =====================================================

void reportTenPesoResult()
{
  Serial.println(F("--------------------------------------"));
  Serial.println(F("PHP 10 DISPENSER LOG"));

  Serial.println(F("EXPECTED: 1"));

  Serial.print(F("IR DETECTED: "));
  Serial.println(detected10);

  if (detected10 == 1)
  {
    Serial.println(F("RESULT: OK"));
  }
  else
  {
    Serial.println(F("RESULT: FAULT"));
  }

  Serial.println(F("--------------------------------------"));
}


// =====================================================
// REPORT ₱20 RESULT
// =====================================================

void reportTwentyPesoResult()
{
  Serial.println(F("--------------------------------------"));
  Serial.println(F("PHP 20 DISPENSER LOG"));

  Serial.println(F("EXPECTED: 1"));

  Serial.print(F("IR DETECTED: "));
  Serial.println(detected20);

  if (detected20 == 1)
  {
    Serial.println(F("RESULT: OK"));
  }
  else
  {
    Serial.println(F("RESULT: FAULT"));
  }

  Serial.println(F("--------------------------------------"));
}


// =====================================================
// ₱1 DISPENSER
// =====================================================
//
// Example: dispenseOnePeso(2)
//
// Servo:
// 30° → 0° → WAIT
// 30° → 0° → WAIT
//
// IR:
// Only monitors what actually comes out.
//
// =====================================================

void dispenseOnePeso(int quantity)
{
  if (quantity <= 0)
    return;


  // Reset monitoring for this dispensing operation

  detected1 = 0;


  Serial.println();
  Serial.println(F("======================================"));

  Serial.print(F("DISPENSING PHP 1 x "));
  Serial.println(quantity);

  Serial.println(F("IR MODE: MONITOR ONLY"));

  Serial.println(F("======================================"));


  // ===================================================
  // DISPENSE REQUIRED NUMBER OF COINS
  // ===================================================

  for (int i = 0; i < quantity; i++)
  {
    Serial.print(F("PHP 1 PUSH "));
    Serial.print(i + 1);
    Serial.print(F(" OF "));
    Serial.println(quantity);


    // =================================================
    // PUSH
    // =================================================

    Serial.println(F("SERVO D8 -> 30"));

    servo1.write(PUSH_1_ANGLE);

    delay(COIN_PUSH_HOLD);


    // =================================================
    // HOME
    // =================================================

    Serial.println(F("SERVO D8 -> 0"));

    servo1.write(HOME_ANGLE);


    // =================================================
    // WAIT BEFORE NEXT COIN
    // =================================================

    if (i < quantity - 1)
    {
      Serial.println(F("WAITING BEFORE NEXT PHP 1..."));

      delay(COIN_REPEAT_DELAY);
    }
  }


  // ===================================================
  // GIVE IR MONITOR A MOMENT TO REGISTER LAST COIN
  // ===================================================

  delay(200);


  // ===================================================
  // REPORT
  // ===================================================

  reportOnePesoResult(quantity);

  Serial.println(F("STATUS:PHP 1 DISPENSING COMPLETE"));
}


// =====================================================
// ₱5 DISPENSER
// =====================================================

void dispenseFivePeso()
{
  detected5 = 0;

  Serial.println(F("STATUS:DISPENSING PHP 5"));

  servo5.write(PUSH_5_ANGLE);

  delay(COIN_PUSH_HOLD);

  servo5.write(HOME_ANGLE);

  delay(200);

  reportFivePesoResult();

  Serial.println(F("STATUS:PHP 5 DISPENSED"));
}


// =====================================================
// ₱10 DISPENSER
// =====================================================

void dispenseTenPeso()
{
  detected10 = 0;

  Serial.println(F("STATUS:DISPENSING PHP 10"));

  servo10.write(PUSH_10_ANGLE);

  delay(COIN_PUSH_HOLD);

  servo10.write(HOME_ANGLE);

  delay(200);

  reportTenPesoResult();

  Serial.println(F("STATUS:PHP 10 DISPENSED"));
}


// =====================================================
// ₱20 DISPENSER
// =====================================================

void dispenseTwentyPeso()
{
  detected20 = 0;

  Serial.println(F("STATUS:DISPENSING PHP 20"));

  servo20.write(PUSH_20_ANGLE);

  delay(COIN_PUSH_HOLD);

  servo20.write(HOME_ANGLE);

  delay(200);

  reportTwentyPesoResult();

  Serial.println(F("STATUS:PHP 20 DISPENSED"));
}


// =====================================================
// ALL SERVOS HOME
// =====================================================

void coinDispensersHome()
{
  servo1.write(HOME_ANGLE);
  servo5.write(HOME_ANGLE);
  servo10.write(HOME_ANGLE);
  servo20.write(HOME_ANGLE);

  Serial.println(F("ALL COIN SERVOS -> HOME"));
}