#include <limits.h>
#include <string.h>
#include <ctype.h>

// Arduino Uno long and WinForms Int32 have the same monetary range.
long paymentTotal = 0;
long paymentInserted = 0;
long paymentBalance = 0;
long paymentChange = 0;
volatile bool paymentActive = false;

bool coinAcceptorReady = false;
bool billAcceptorReady = false;
bool coinDispenserReady = false;
bool billDispenserReady = false;

long changeRequested = 0;
long changeDispensed = 0;
long changeRemaining = 0;
long verifiedDispensedAmount = 0;
bool dispensingBusy = false;
bool dispensingAborted = false;

const byte COMMAND_SIZE = 48;
const byte COMMAND_QUEUE_DEPTH = 4;
char serialCommand[COMMAND_SIZE];
byte serialCommandLength = 0;
bool serialCommandOverflow = false;
char commandQueue[COMMAND_QUEUE_DEPTH][COMMAND_SIZE];
byte commandQueueCount = 0;
bool commandQueueRejected = false;
bool commandTooLong = false;
const byte SERIAL_SERVICE_BUDGET = 32;

void setup()
{
  Serial.begin(9600);
  setupBillAcceptor();
  setupCoinAcceptor();
  setupBillDispenser();
  setupCoinDispenser();
  delay(500);
  coinAcceptorReady = true;
  billAcceptorReady = true;
  coinDispenserReady = true;
  billDispenserReady = true;
  resetPayment();
  sendHardwareStatus();
}

void loop()
{
  processSerialCommands();
  updateCoinAcceptor();
  updateBillAcceptor();
  updateCoinSensors();
}

void cancelQueuedDispenses()
{
  byte retained = 0;
  for (byte i = 0; i < commandQueueCount; ++i)
  {
    if (strncmp(commandQueue[i], "DISPENSE:", 9) == 0)
      Serial.println(F("STATUS:QUEUED DISPENSE CANCELLED"));
    else
    {
      if (retained != i)
        strcpy(commandQueue[retained], commandQueue[i]);
      ++retained;
    }
  }
  commandQueueCount = retained;
}

void stopDispensing()
{
  dispensingAborted = true;
  stopBillMotors();
  coinDispensersHome();
  cancelQueuedDispenses();
  if (dispensingBusy)
    Serial.println(F("STATUS:PAYOUT INTERRUPTED"));
}

void receiveSerialCommand(const char *command)
{
  // Safety commands never wait behind a payout or the deferred queue.
  if (strcmp(command, "STOPDISPENSERS") == 0)
  {
    stopDispensing();
    Serial.println(F("STATUS:DISPENSERS STOPPED"));
  }
  else if (strcmp(command, "RESET") == 0)
  {
    stopDispensing();
    resetPayment();
  }
  else if (!dispensingBusy && strcmp(command, "GETSTATUS") == 0)
  {
    sendHardwareStatus();
    sendPaymentStatus();
  }
  else if (dispensingBusy || commandQueueCount > 0)
  {
    if (dispensingAborted && strncmp(command, "DISPENSE:", 9) == 0)
      Serial.println(F("STATUS:DISPENSE CANCELLED"));
    else if (commandQueueCount < COMMAND_QUEUE_DEPTH)
      strcpy(commandQueue[commandQueueCount++], command);
    else
      commandQueueRejected = true;
  }
  else
    handleSerialCommand(command);
}

void processSerialCommands()
{
  byte serviced = 0;
  while (Serial.available() > 0 && serviced++ < SERIAL_SERVICE_BUDGET)
  {
    char c = Serial.read();
    if (c == '\n' || c == '\r')
    {
      bool overflow = serialCommandOverflow;
      byte length = serialCommandLength;
      char command[COMMAND_SIZE];
      serialCommand[length] = '\0';
      strcpy(command, serialCommand);
      char *trimmed = command;
      while (*trimmed && isspace((unsigned char)*trimmed)) ++trimmed;
      if (trimmed != command) memmove(command, trimmed, strlen(trimmed) + 1);
      size_t trimmedLength = strlen(command);
      while (trimmedLength > 0 && isspace((unsigned char)command[trimmedLength - 1]))
        command[--trimmedLength] = '\0';
      // Reset BEFORE dispatch; nested servicing may collect a partial next line.
      serialCommandLength = 0;
      serialCommandOverflow = false;
      if (overflow)
        commandTooLong = true;
      else if (trimmedLength > 0)
      {
        receiveSerialCommand(command);
      }
    }
    else if (!serialCommandOverflow)
    {
      if (serialCommandLength < COMMAND_SIZE - 1)
        serialCommand[serialCommandLength++] = c;
      else
        serialCommandOverflow = true;
    }
  }

  if (!dispensingBusy && commandQueueCount > 0)
  {
    char command[COMMAND_SIZE];
    strcpy(command, commandQueue[0]);
    for (byte i = 1; i < commandQueueCount; ++i)
      strcpy(commandQueue[i - 1], commandQueue[i]);
    --commandQueueCount;
    handleSerialCommand(command);
  }
  if (!dispensingBusy)
    reportCommandFaults();
}

void reportCommandFaults()
{
  if (commandQueueRejected)
  {
    Serial.println(F("STATUS:COMMAND QUEUE FULL"));
    commandQueueRejected = false;
  }
  if (commandTooLong)
  {
    Serial.println(F("STATUS:COMMAND TOO LONG"));
    commandTooLong = false;
  }
}

bool parsePositiveAmount(const char *text, long &value)
{
  value = 0;
  if (*text == '\0') return false;
  while (*text)
  {
    if (*text < '0' || *text > '9') return false;
    byte digit = *text++ - '0';
    if (value > (LONG_MAX - digit) / 10) return false;
    value = value * 10 + digit;
  }
  return value > 0;
}

void handleSerialCommand(const char *command)
{
  long amount;
  if (strcmp(command, "GETSTATUS") == 0)
  {
    sendHardwareStatus();
    sendPaymentStatus();
  }
  else if (strncmp(command, "SETTOTAL:", 9) == 0)
  {
    if (parsePositiveAmount(command + 9, amount)) startPayment(amount);
    else Serial.println(F("STATUS:INVALID TOTAL"));
  }
  else if (strncmp(command, "BILL:", 5) == 0)
  {
    if (!parsePositiveAmount(command + 5, amount) ||
        (amount != 50 && amount != 100 && amount != 500 && amount != 1000))
      Serial.println(F("STATUS:INVALID BILL"));
    else if (!paymentActive)
      Serial.println(F("STATUS:PAYMENT NOT ACTIVE"));
    else
    {
      addPayment((int)amount);
      Serial.print(F("BILL ACCEPTED:"));
      Serial.println(amount);
    }
  }
  else if (strncmp(command, "DISPENSE:", 9) == 0)
  {
    if (!parsePositiveAmount(command + 9, amount) ||
        (amount != 1 && amount != 5 && amount != 10 && amount != 20 &&
         amount != 50 && amount != 100))
      Serial.println(F("STATUS:INVALID DISPENSE"));
    else if (changeRemaining > 0 && amount > changeRemaining)
      Serial.println(F("STATUS:DISPENSE EXCEEDS OUTSTANDING CHANGE"));
    else
      dispenseManual((byte)amount);
  }
  else
    Serial.println(F("STATUS:UNKNOWN COMMAND"));
}

void sendHardwareStatus()
{
  Serial.println(coinAcceptorReady ? F("COIN_ACCEPTOR:READY") : F("COIN_ACCEPTOR:NOT_READY"));
  Serial.println(billAcceptorReady ? F("BILL_ACCEPTOR:READY") : F("BILL_ACCEPTOR:NOT_READY"));
  Serial.println(coinDispenserReady ? F("COIN_DISPENSER:READY") : F("COIN_DISPENSER:NOT_READY"));
  Serial.println(billDispenserReady ? F("BILL_DISPENSER:READY") : F("BILL_DISPENSER:NOT_READY"));
}

void sendChangeStatus()
{
  Serial.print(F("CHANGE_REQUESTED:")); Serial.println(changeRequested);
  Serial.print(F("CHANGE_DISPENSED:")); Serial.println(changeDispensed);
  Serial.print(F("CHANGE_REMAINING:")); Serial.println(changeRemaining);
}

void sendPaymentStatus()
{
  Serial.print(F("TOTAL:")); Serial.println(paymentTotal);
  Serial.print(F("PAYMENT:")); Serial.println(paymentInserted);
  Serial.print(F("BALANCE:")); Serial.println(paymentBalance);
  Serial.print(F("CHANGE:")); Serial.println(paymentChange);
  sendChangeStatus();
}

void startPayment(long total)
{
  if (changeRemaining > 0)
  {
    Serial.println(F("STATUS:OUTSTANDING CHANGE - PAYMENT NOT STARTED"));
    sendChangeStatus();
    return;
  }
  paymentTotal = total;
  paymentInserted = 0;
  paymentBalance = total;
  paymentChange = 0;
  changeRequested = changeDispensed = changeRemaining = 0;
  paymentActive = true;
  clearCoinPulses();
  sendPaymentStatus();
  Serial.println(F("STATUS:WAITING FOR PAYMENT"));
}

void resetPayment()
{
  paymentTotal = paymentInserted = paymentBalance = 0;
  paymentActive = false;
  clearCoinPulses();
  // RESET must not erase money still owed, including an interrupted payout.
  if (!dispensingBusy && changeRemaining == 0)
    changeRequested = changeDispensed = 0;
  paymentChange = changeRemaining;
  sendPaymentStatus();
  Serial.println(changeRemaining > 0 ? F("STATUS:CHANGE INCOMPLETE") : F("STATUS:READY"));
}

void addPayment(int amount)
{
  if (!paymentActive || amount <= 0) return;
  if (paymentInserted > LONG_MAX - amount)
  {
    Serial.println(F("STATUS:PAYMENT AMOUNT OVERFLOW"));
    return;
  }
  paymentInserted += amount;
  if (paymentInserted < paymentTotal)
  {
    paymentBalance = paymentTotal - paymentInserted;
    sendPaymentStatus();
    Serial.println(F("STATUS:WAITING FOR PAYMENT"));
    return;
  }
  paymentBalance = 0;
  paymentActive = false;
  changeRequested = paymentInserted - paymentTotal;
  changeDispensed = 0;
  changeRemaining = paymentChange = changeRequested;
  sendPaymentStatus();
  Serial.println(F("STATUS:PAYMENT COMPLETE"));
  if (changeRemaining > 0) dispenseChange();
}

bool dispenseDenomination(byte denomination)
{
  verifiedDispensedAmount = 0;
  switch (denomination)
  {
    case 1: return dispenseOnePeso(1);
    case 5: return dispenseFivePeso();
    case 10: return dispenseTenPeso();
    case 20: return dispenseTwentyPeso();
    case 50: return dispenseBill50();
    case 100: return dispenseBill100();
  }
  return false;
}

void accountVerifiedChange()
{
  // A confirmed exit just before STOP/RESET is still money actually paid out.
  if (verifiedDispensedAmount > 0 && changeRemaining > 0)
  {
    changeDispensed += verifiedDispensedAmount;
    changeRemaining = changeRequested - changeDispensed;
    if (changeRemaining < 0) changeRemaining = 0;
    paymentChange = changeRemaining;
    Serial.print(F("CHANGE:")); Serial.println(paymentChange);
    sendChangeStatus();
  }
}

void finishDispensing()
{
  stopBillMotors();
  coinDispensersHome();
  dispensingBusy = false;
  reportCommandFaults();
}

void dispenseManual(byte denomination)
{
  dispensingBusy = true;
  dispensingAborted = false;
  bool payingOutstandingChange = changeRemaining > 0;
  bool success = dispenseDenomination(denomination);
  if (payingOutstandingChange) accountVerifiedChange();
  finishDispensing();
  Serial.print(F("DISPENSE:")); Serial.print(denomination);
  Serial.println(success && !dispensingAborted ? F(":COMPLETE") : F(":FAILED"));
  if (payingOutstandingChange)
    Serial.println(success && !dispensingAborted && changeRemaining == 0 ?
      F("STATUS:CHANGE DISPENSE COMPLETE") : F("STATUS:CHANGE INCOMPLETE"));
}

void dispenseChange()
{
  dispensingBusy = true;
  dispensingAborted = false;
  Serial.println(F("STATUS:DISPENSING CHANGE"));
  bool success = true;
  while (changeRemaining > 0 && !dispensingAborted)
  {
    byte denomination = changeRemaining >= 100 ? 100 : changeRemaining >= 50 ? 50 :
      changeRemaining >= 20 ? 20 : changeRemaining >= 10 ? 10 : changeRemaining >= 5 ? 5 : 1;
    success = dispenseDenomination(denomination);
    accountVerifiedChange();
    if (!success || dispensingAborted) break;
  }
  finishDispensing();
  sendChangeStatus();
  Serial.println(success && !dispensingAborted && changeRemaining == 0 ?
    F("STATUS:CHANGE DISPENSE COMPLETE") : F("STATUS:CHANGE INCOMPLETE"));
}
