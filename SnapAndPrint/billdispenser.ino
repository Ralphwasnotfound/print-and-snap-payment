const byte BILL50_IN1 = 7;
const byte BILL50_IN2 = 6;
const byte BILL100_IN3 = 5;
const byte BILL100_IN4 = 4;
const byte BILL50_IR = 12;
const byte BILL100_IR = 13;
const unsigned long BILL_SENSOR_TIMEOUT = 5000;

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

void stopBillMotors()
{
  digitalWrite(BILL50_IN1, LOW);
  digitalWrite(BILL50_IN2, LOW);
  digitalWrite(BILL100_IN3, LOW);
  digitalWrite(BILL100_IN4, LOW);
}

bool dispenseBill(byte denomination)
{
  stopBillMotors();
  if (dispensingAborted) return false;
  byte sensor = denomination == 50 ? BILL50_IR : BILL100_IR;
  byte forward = denomination == 50 ? BILL50_IN1 : BILL100_IN3;
  byte reverse = denomination == 50 ? BILL50_IN2 : BILL100_IN4;
  if (digitalRead(sensor) != HIGH)
  {
    Serial.print(F("BILL")); Serial.print(denomination); Serial.println(F(":FAULT"));
    Serial.println(F("STATUS:BILL SENSOR BLOCKED BEFORE DISPENSE"));
    return false;
  }

  Serial.print(F("STATUS:DISPENSING PHP ")); Serial.println(denomination);
  digitalWrite(forward, HIGH);
  digitalWrite(reverse, LOW);
  unsigned long started = millis();
  while (millis() - started < BILL_SENSOR_TIMEOUT)
  {
    processSerialCommands();
    if (dispensingAborted)
    {
      stopBillMotors();
      return false;
    }
    updateCoinSensors();
    if (millis() - started >= BILL_SENSOR_TIMEOUT) break;
    // The preflight HIGH and this fresh LOW form the verified exit edge.
    if (digitalRead(sensor) == LOW)
    {
      stopBillMotors();
      verifiedDispensedAmount += denomination;
      Serial.print(F("BILL")); Serial.print(denomination); Serial.println(F(":SENSOR"));
      Serial.print(F("STATUS:PHP ")); Serial.print(denomination); Serial.println(F(" DISPENSED"));
      return true;
    }
  }
  stopBillMotors();
  Serial.print(F("BILL")); Serial.print(denomination); Serial.println(F(":FAULT"));
  Serial.print(F("STATUS:PHP ")); Serial.print(denomination); Serial.println(F(" DISPENSER TIMEOUT"));
  return false;
}

bool dispenseBill50() { return dispenseBill(50); }
bool dispenseBill100() { return dispenseBill(100); }
