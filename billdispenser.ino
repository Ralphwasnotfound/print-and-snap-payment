// =====================================================
// SNAP AND PRINT
// BILL DISPENSER
// =====================================================
//
// ₱50:
// IN1 = D7
// IN2 = D6
// IR  = D12
//
// ₱100:
// IN3 = D5
// IN4 = D4
// IR  = D13
//
// Controlled by PrintAndSnap.ino:
//
// DISPENSE:50
// DISPENSE:100
// STOPDISPENSERS
//
// =====================================================


// =====================================================
// ₱50 MOTOR
// =====================================================

const byte BILL50_IN1 = 7;
const byte BILL50_IN2 = 6;


// =====================================================
// ₱100 MOTOR
// =====================================================

const byte BILL100_IN3 = 5;
const byte BILL100_IN4 = 4;


// =====================================================
// BILL IR SENSORS
// =====================================================

const byte BILL50_IR  = 12;
const byte BILL100_IR = 13;


// =====================================================
// SAFETY TIMEOUT
// =====================================================

const unsigned long BILL_SENSOR_TIMEOUT = 5000;


// =====================================================
// INITIALIZE
// =====================================================

void setupBillDispenser()
{
  pinMode(BILL50_IN1, OUTPUT);
  pinMode(BILL50_IN2, OUTPUT);

  pinMode(BILL100_IN3, OUTPUT);
  pinMode(BILL100_IN4, OUTPUT);

  pinMode(BILL50_IR, INPUT);
  pinMode(BILL100_IR, INPUT);

  stopBillMotors();
}


// =====================================================
// STOP ALL BILL MOTORS
// =====================================================

void stopBillMotors()
{
  digitalWrite(BILL50_IN1, LOW);
  digitalWrite(BILL50_IN2, LOW);

  digitalWrite(BILL100_IN3, LOW);
  digitalWrite(BILL100_IN4, LOW);
}


// =====================================================
// DISPENSE ₱50
// =====================================================

void dispenseBill50()
{
  stopBillMotors();

  Serial.println(F("STATUS:DISPENSING PHP 50"));

  // Start motor
  digitalWrite(BILL50_IN1, HIGH);
  digitalWrite(BILL50_IN2, LOW);

  unsigned long startTime = millis();


  while (millis() - startTime < BILL_SENSOR_TIMEOUT)
  {
    // Emergency stop command
    if (Serial.available() > 0)
    {
      char command = Serial.read();

      if (command == '0')
      {
        stopBillMotors();

        Serial.println(F("STATUS:DISPENSER STOPPED"));

        return;
      }
    }


    // Bill detected
    if (digitalRead(BILL50_IR) == LOW)
    {
      stopBillMotors();

      Serial.println(F("BILL50:SENSOR"));
      Serial.println(F("STATUS:PHP 50 DISPENSED"));

      return;
    }
  }


  // Timeout
  stopBillMotors();

  Serial.println(F("STATUS:PHP 50 DISPENSER TIMEOUT"));
}


// =====================================================
// DISPENSE ₱100
// =====================================================

void dispenseBill100()
{
  stopBillMotors();

  Serial.println(F("STATUS:DISPENSING PHP 100"));

  // Start motor
  digitalWrite(BILL100_IN3, HIGH);
  digitalWrite(BILL100_IN4, LOW);

  unsigned long startTime = millis();


  while (millis() - startTime < BILL_SENSOR_TIMEOUT)
  {
    // Emergency stop command
    if (Serial.available() > 0)
    {
      char command = Serial.read();

      if (command == '0')
      {
        stopBillMotors();

        Serial.println(F("STATUS:DISPENSER STOPPED"));

        return;
      }
    }


    // Bill detected
    if (digitalRead(BILL100_IR) == LOW)
    {
      stopBillMotors();

      Serial.println(F("BILL100:SENSOR"));
      Serial.println(F("STATUS:PHP 100 DISPENSED"));

      return;
    }
  }


  // Timeout
  stopBillMotors();

  Serial.println(F("STATUS:PHP 100 DISPENSER TIMEOUT"));
}