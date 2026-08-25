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
// Controlled by PrintAndSnap.ino:
//
// DISPENSE:1
// DISPENSE:5
// DISPENSE:10
// DISPENSE:20
// STOPDISPENSERS
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
// SENSOR TIMEOUT
// =====================================================

const unsigned long SENSOR_TIMEOUT = 1500;


// =====================================================
// INITIALIZE
// =====================================================

void setupCoinDispenser()
{
  servo1.attach(SERVO_1_PIN);
  servo5.attach(SERVO_5_PIN);
  servo10.attach(SERVO_10_PIN);
  servo20.attach(SERVO_20_PIN);


  pinMode(IR_1_PIN, INPUT);
  pinMode(IR_5_PIN, INPUT);
  pinMode(IR_10_PIN, INPUT);
  pinMode(IR_20_PIN, INPUT);


  coinDispensersHome();

  delay(500);
}


// =====================================================
// ₱1
// =====================================================

void dispenseOnePeso()
{
  Serial.println(F("STATUS:DISPENSING PHP 1"));

  servo1.write(PUSH_1_ANGLE);

  unsigned long startTime = millis();

  bool coinDetected = false;


  while (millis() - startTime < SENSOR_TIMEOUT)
  {
    if (digitalRead(IR_1_PIN) == LOW)
    {
      coinDetected = true;
      break;
    }
  }


  servo1.write(HOME_ANGLE);


  if (coinDetected)
  {
    Serial.println(F("COIN1:SENSOR"));
    Serial.println(F("STATUS:PHP 1 DISPENSED"));
  }
  else
  {
    Serial.println(F("STATUS:PHP 1 DISPENSER TIMEOUT"));
  }
}


// =====================================================
// ₱5
// =====================================================

void dispenseFivePeso()
{
  Serial.println(F("STATUS:DISPENSING PHP 5"));

  servo5.write(PUSH_5_ANGLE);

  unsigned long startTime = millis();

  bool coinDetected = false;


  while (millis() - startTime < SENSOR_TIMEOUT)
  {
    if (digitalRead(IR_5_PIN) == LOW)
    {
      coinDetected = true;
      break;
    }
  }


  servo5.write(HOME_ANGLE);


  if (coinDetected)
  {
    Serial.println(F("COIN5:SENSOR"));
    Serial.println(F("STATUS:PHP 5 DISPENSED"));
  }
  else
  {
    Serial.println(F("STATUS:PHP 5 DISPENSER TIMEOUT"));
  }
}


// =====================================================
// ₱10
// =====================================================

void dispenseTenPeso()
{
  Serial.println(F("STATUS:DISPENSING PHP 10"));

  servo10.write(PUSH_10_ANGLE);

  unsigned long startTime = millis();

  bool coinDetected = false;


  while (millis() - startTime < SENSOR_TIMEOUT)
  {
    if (digitalRead(IR_10_PIN) == LOW)
    {
      coinDetected = true;
      break;
    }
  }


  servo10.write(HOME_ANGLE);


  if (coinDetected)
  {
    Serial.println(F("COIN10:SENSOR"));
    Serial.println(F("STATUS:PHP 10 DISPENSED"));
  }
  else
  {
    Serial.println(F("STATUS:PHP 10 DISPENSER TIMEOUT"));
  }
}


// =====================================================
// ₱20
// =====================================================

void dispenseTwentyPeso()
{
  Serial.println(F("STATUS:DISPENSING PHP 20"));

  servo20.write(PUSH_20_ANGLE);

  unsigned long startTime = millis();

  bool coinDetected = false;


  while (millis() - startTime < SENSOR_TIMEOUT)
  {
    if (digitalRead(IR_20_PIN) == LOW)
    {
      coinDetected = true;
      break;
    }
  }


  servo20.write(HOME_ANGLE);


  if (coinDetected)
  {
    Serial.println(F("COIN20:SENSOR"));
    Serial.println(F("STATUS:PHP 20 DISPENSED"));
  }
  else
  {
    Serial.println(F("STATUS:PHP 20 DISPENSER TIMEOUT"));
  }
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
}