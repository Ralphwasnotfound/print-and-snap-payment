#include <Servo.h>

Servo servo1;
Servo servo5;
Servo servo10;
Servo servo20;

const byte SERVO_1_PIN = 8;
const byte SERVO_5_PIN = 9;
const byte SERVO_10_PIN = 10;
const byte SERVO_20_PIN = 11;
const byte IR_1_PIN = A1;
const byte IR_5_PIN = A2;
const byte IR_10_PIN = A3;
const byte IR_20_PIN = A4;
const byte HOME_ANGLE = 0;
const byte PUSH_1_ANGLE = 30;
const byte PUSH_5_ANGLE = 40;
const byte PUSH_10_ANGLE = 40;
const byte PUSH_20_ANGLE = 40;
const unsigned long COIN_PUSH_HOLD = 500;
const unsigned long COIN_REPEAT_DELAY = 500;
const unsigned long IR_DEBOUNCE = 100;
// Includes the unchanged push and settling intervals; requires bench calibration.
const unsigned long COIN_SENSOR_TIMEOUT = 2000;

volatile unsigned int detected1 = 0;
volatile unsigned int detected5 = 0;
volatile unsigned int detected10 = 0;
volatile unsigned int detected20 = 0;
byte lastIR1State;
byte lastIR5State;
byte lastIR10State;
byte lastIR20State;
unsigned long lastIR1Time = 0;
unsigned long lastIR5Time = 0;
unsigned long lastIR10Time = 0;
unsigned long lastIR20Time = 0;

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
  lastIR1State = digitalRead(IR_1_PIN);
  lastIR5State = digitalRead(IR_5_PIN);
  lastIR10State = digitalRead(IR_10_PIN);
  lastIR20State = digitalRead(IR_20_PIN);
  coinDispensersHome();
  delay(500);
  Serial.println(F("COIN_DISPENSER:READY"));
}

void monitorCoinSensor(byte pin, byte &previous, unsigned long &lastTime,
                       volatile unsigned int &count)
{
  byte state = digitalRead(pin);
  unsigned long now = millis();
  if (state != previous)
  {
    previous = state;
    if (state == LOW && now - lastTime >= IR_DEBOUNCE)
    {
      ++count;
      lastTime = now;
    }
  }
}

void updateCoinSensors()
{
  monitorCoinSensor(IR_1_PIN, lastIR1State, lastIR1Time, detected1);
  monitorCoinSensor(IR_5_PIN, lastIR5State, lastIR5Time, detected5);
  monitorCoinSensor(IR_10_PIN, lastIR10State, lastIR10Time, detected10);
  monitorCoinSensor(IR_20_PIN, lastIR20State, lastIR20Time, detected20);
}

void writeCoinServo(byte denomination, byte angle)
{
  switch (denomination)
  {
    case 1: servo1.write(angle); break;
    case 5: servo5.write(angle); break;
    case 10: servo10.write(angle); break;
    case 20: servo20.write(angle); break;
  }
}

bool sampleCoinExit(byte sensor, bool &blocked, bool &confirmed, byte denomination)
{
  byte state = digitalRead(sensor);
  if (confirmed && state == LOW)
  {
    Serial.println(F("STATUS:COIN SENSOR REBLOCKED"));
    return false;
  }
  if (state == LOW) blocked = true;
  else if (blocked && !confirmed)
  {
    confirmed = true;
    verifiedDispensedAmount += denomination;
  }
  return true;
}

bool waitCoinInterval(unsigned long duration, byte sensor, bool &blocked,
                      bool &confirmed, byte denomination)
{
  unsigned long started = millis();
  while (millis() - started < duration)
  {
    processSerialCommands();
    if (dispensingAborted) return false;
    updateCoinSensors();
    if (!sampleCoinExit(sensor, blocked, confirmed, denomination)) return false;
  }
  return !dispensingAborted;
}

bool dispenseCoin(byte denomination)
{
  if (dispensingAborted) return false;
  byte sensor = denomination == 1 ? IR_1_PIN : denomination == 5 ? IR_5_PIN :
                denomination == 10 ? IR_10_PIN : IR_20_PIN;
  byte angle = denomination == 1 ? PUSH_1_ANGLE : denomination == 5 ? PUSH_5_ANGLE :
               denomination == 10 ? PUSH_10_ANGLE : PUSH_20_ANGLE;
  if (digitalRead(sensor) != HIGH)
  {
    Serial.println(F("STATUS:COIN SENSOR BLOCKED BEFORE DISPENSE"));
    return false;
  }

  bool blocked = false;
  bool confirmed = false;
  unsigned long started = millis();
  writeCoinServo(denomination, angle);
  bool safe = waitCoinInterval(COIN_PUSH_HOLD, sensor, blocked, confirmed, denomination);
  writeCoinServo(denomination, HOME_ANGLE);
  if (!safe) return false;
  if (!waitCoinInterval(200, sensor, blocked, confirmed, denomination)) return false;

  while (!confirmed && millis() - started < COIN_SENSOR_TIMEOUT)
  {
    processSerialCommands();
    if (dispensingAborted) return false;
    updateCoinSensors();
    if (!sampleCoinExit(sensor, blocked, confirmed, denomination)) return false;
  }
  if (!confirmed)
    Serial.println(F("STATUS:COIN DISPENSER TIMEOUT"));
  return confirmed && !dispensingAborted;
}

bool dispenseOnePeso(int quantity)
{
  if (quantity <= 0) return false;
  for (int i = 0; i < quantity; ++i)
  {
    if (!dispenseCoin(1)) return false;
    if (i < quantity - 1)
    {
      unsigned long started = millis();
      while (millis() - started < COIN_REPEAT_DELAY)
      {
        processSerialCommands();
        if (dispensingAborted) return false;
        updateCoinSensors();
      }
    }
  }
  return true;
}

bool dispenseFivePeso() { return dispenseCoin(5); }
bool dispenseTenPeso() { return dispenseCoin(10); }
bool dispenseTwentyPeso() { return dispenseCoin(20); }

void coinDispensersHome()
{
  servo1.write(HOME_ANGLE);
  servo5.write(HOME_ANGLE);
  servo10.write(HOME_ANGLE);
  servo20.write(HOME_ANGLE);
}
