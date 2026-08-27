// =====================================================
// SNAP AND PRINT
// MAIN PAYMENT CONTROLLER / BRAIN
// =====================================================


// =====================================================
// PAYMENT VARIABLES
// =====================================================

int paymentTotal = 0;
int paymentInserted = 0;
int paymentBalance = 0;
int paymentChange = 0;

bool paymentActive = false;


// =====================================================
// HARDWARE STATUS
// =====================================================

bool coinAcceptorReady = false;
bool billAcceptorReady = false;
bool coinDispenserReady = false;
bool billDispenserReady = false;


// =====================================================
// SERIAL COMMAND BUFFER
// =====================================================

String serialCommand = "";


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(9600);

  // ---------------------------------------------------
  // Initialize hardware modules
  // ---------------------------------------------------

  setupBillAcceptor();
  setupCoinAcceptor();

  setupBillDispenser();
  setupCoinDispenser();

  delay(500);


  // ---------------------------------------------------
  // Hardware modules initialized
  // ---------------------------------------------------

  coinAcceptorReady = true;
  billAcceptorReady = true;
  coinDispenserReady = true;
  billDispenserReady = true;


  // ---------------------------------------------------
  // Reset payment
  // ---------------------------------------------------

  resetPayment();


  // ---------------------------------------------------
  // Tell C# that hardware is ready
  // ---------------------------------------------------

  sendHardwareStatus();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // ---------------------------------------------------
  // Receive commands from C#
  // ---------------------------------------------------

  processSerialCommands();


  // ---------------------------------------------------
  // Monitor payment hardware
  // ---------------------------------------------------

  updateCoinAcceptor();
  updateBillAcceptor();


  // ---------------------------------------------------
  // Monitor coin dispenser IR sensors
  // ---------------------------------------------------

  updateCoinSensors();
}


// =====================================================
// SERIAL COMMAND PROCESSOR
// =====================================================

void processSerialCommands()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();


    // -------------------------------------------------
    // Command finished
    // -------------------------------------------------

    if (c == '\n' || c == '\r')
    {
      if (serialCommand.length() > 0)
      {
        handleSerialCommand(serialCommand);

        serialCommand = "";
      }
    }
    else
    {
      serialCommand += c;
    }
  }
}


// =====================================================
// SERIAL COMMAND HANDLER
// =====================================================

void handleSerialCommand(String command)
{
  command.trim();


  // ===================================================
  // SET TOTAL
  // ===================================================

  if (command.startsWith("SETTOTAL:"))
  {
    String value = command.substring(9);

    int total = value.toInt();

    if (total > 0)
    {
      startPayment(total);
    }
    else
    {
      Serial.println(F("STATUS:INVALID TOTAL"));
    }

    return;
  }


  // ===================================================
  // MANUAL BILL PAYMENT
  // ===================================================

  if (command.startsWith("BILL:"))
  {
    String value = command.substring(5);

    int amount = value.toInt();

    if (amount == 50 ||
        amount == 100 ||
        amount == 500 ||
        amount == 1000)
    {
      if (paymentActive)
      {
        addPayment(amount);

        Serial.print(F("BILL ACCEPTED:"));
        Serial.println(amount);
      }
      else
      {
        Serial.println(F("STATUS:PAYMENT NOT ACTIVE"));
      }
    }
    else
    {
      Serial.println(F("STATUS:INVALID BILL"));
    }

    return;
  }


  // ===================================================
  // RESET
  // ===================================================

  if (command == "RESET")
  {
    resetPayment();

    return;
  }


  // ===================================================
  // GET STATUS
  // ===================================================

  if (command == "GETSTATUS")
  {
    sendHardwareStatus();
    sendPaymentStatus();

    return;
  }


  // ===================================================
  // DISPENSE ₱1
  // ===================================================

  if (command == "DISPENSE:1")
  {
    dispenseOnePeso(1);

    Serial.println(F("DISPENSE:1:COMPLETE"));

    return;
  }


  // ===================================================
  // DISPENSE ₱5
  // ===================================================

  if (command == "DISPENSE:5")
  {
    dispenseFivePeso();

    Serial.println(F("DISPENSE:5:COMPLETE"));

    return;
  }


  // ===================================================
  // DISPENSE ₱10
  // ===================================================

  if (command == "DISPENSE:10")
  {
    dispenseTenPeso();

    Serial.println(F("DISPENSE:10:COMPLETE"));

    return;
  }


  // ===================================================
  // DISPENSE ₱20
  // ===================================================

  if (command == "DISPENSE:20")
  {
    dispenseTwentyPeso();

    Serial.println(F("DISPENSE:20:COMPLETE"));

    return;
  }


  // ===================================================
  // DISPENSE ₱50
  // ===================================================

  if (command == "DISPENSE:50")
  {
    dispenseBill50();

    Serial.println(F("DISPENSE:50:COMPLETE"));

    return;
  }


  // ===================================================
  // DISPENSE ₱100
  // ===================================================

  if (command == "DISPENSE:100")
  {
    dispenseBill100();

    Serial.println(F("DISPENSE:100:COMPLETE"));

    return;
  }


  // ===================================================
  // STOP DISPENSERS
  // ===================================================

  if (command == "STOPDISPENSERS")
  {
    stopBillMotors();
    coinDispensersHome();

    Serial.println(F("STATUS:DISPENSERS STOPPED"));

    return;
  }


  // ===================================================
  // UNKNOWN COMMAND
  // ===================================================

  Serial.println(F("STATUS:UNKNOWN COMMAND"));
}


// =====================================================
// HARDWARE STATUS
// =====================================================

void sendHardwareStatus()
{
  if (coinAcceptorReady)
  {
    Serial.println(F("COIN_ACCEPTOR:READY"));
  }
  else
  {
    Serial.println(F("COIN_ACCEPTOR:NOT_READY"));
  }


  if (billAcceptorReady)
  {
    Serial.println(F("BILL_ACCEPTOR:READY"));
  }
  else
  {
    Serial.println(F("BILL_ACCEPTOR:NOT_READY"));
  }


  if (coinDispenserReady)
  {
    Serial.println(F("COIN_DISPENSER:READY"));
  }
  else
  {
    Serial.println(F("COIN_DISPENSER:NOT_READY"));
  }


  if (billDispenserReady)
  {
    Serial.println(F("BILL_DISPENSER:READY"));
  }
  else
  {
    Serial.println(F("BILL_DISPENSER:NOT_READY"));
  }
}


// =====================================================
// SEND PAYMENT STATUS
// =====================================================

void sendPaymentStatus()
{
  Serial.print(F("TOTAL:"));
  Serial.println(paymentTotal);

  Serial.print(F("PAYMENT:"));
  Serial.println(paymentInserted);

  Serial.print(F("BALANCE:"));
  Serial.println(paymentBalance);

  Serial.print(F("CHANGE:"));
  Serial.println(paymentChange);
}


// =====================================================
// START PAYMENT
// =====================================================

void startPayment(int total)
{
  paymentTotal = total;

  paymentInserted = 0;

  paymentBalance = total;

  paymentChange = 0;

  paymentActive = true;


  clearCoinPulses();


  sendPaymentStatus();

  Serial.println(F("STATUS:WAITING FOR PAYMENT"));
}


// =====================================================
// RESET PAYMENT
// =====================================================

void resetPayment()
{
  paymentTotal = 0;

  paymentInserted = 0;

  paymentBalance = 0;

  paymentChange = 0;

  paymentActive = false;


  clearCoinPulses();


  Serial.println(F("TOTAL:0"));
  Serial.println(F("PAYMENT:0"));
  Serial.println(F("BALANCE:0"));
  Serial.println(F("CHANGE:0"));

  Serial.println(F("STATUS:READY"));
}


// =====================================================
// ADD PAYMENT
// =====================================================

void addPayment(int amount)
{
  if (!paymentActive)
    return;

  if (amount <= 0)
    return;


  // ---------------------------------------------------
  // Add payment
  // ---------------------------------------------------

  paymentInserted += amount;


  // ===================================================
  // PAYMENT COMPLETE
  // ===================================================

  if (paymentInserted >= paymentTotal)
  {
    paymentBalance = 0;

    paymentChange =
      paymentInserted - paymentTotal;

    paymentActive = false;


    // =================================================
    // SEND FINAL PAYMENT STATUS
    // =================================================

    Serial.print(F("PAYMENT:"));
    Serial.println(paymentInserted);

    Serial.print(F("BALANCE:"));
    Serial.println(paymentBalance);

    Serial.print(F("CHANGE:"));
    Serial.println(paymentChange);

    Serial.println(F("STATUS:PAYMENT COMPLETE"));


    // =================================================
    // AUTOMATIC DYNAMIC CHANGE
    // =================================================

    if (paymentChange > 0)
    {
      Serial.print(F("STATUS:DISPENSING CHANGE PHP "));
      Serial.println(paymentChange);

      dispenseChange(paymentChange);

      Serial.println(F("STATUS:CHANGE DISPENSE COMPLETE"));
    }

    return;
  }


  // ===================================================
  // PAYMENT INCOMPLETE
  // ===================================================

  paymentBalance =
    paymentTotal - paymentInserted;

  paymentChange = 0;


  Serial.print(F("PAYMENT:"));
  Serial.println(paymentInserted);

  Serial.print(F("BALANCE:"));
  Serial.println(paymentBalance);

  Serial.print(F("CHANGE:"));
  Serial.println(paymentChange);

  Serial.println(F("STATUS:WAITING FOR PAYMENT"));
}


// =====================================================
// DISPENSE DYNAMIC CHANGE
// =====================================================
//
// Available denominations:
// ₱20, ₱10, ₱5, ₱1
//
// The actual change amount is calculated dynamically.
// Nothing is hardcoded to a specific amount.
//
// =====================================================

void dispenseChange(int change)
{
  if (change <= 0)
    return;


  int remaining = change;


  Serial.println(F("======================================"));

  Serial.print(F("DISPENSING CHANGE: PHP "));
  Serial.println(change);

  Serial.println(F("======================================"));


  // ===================================================
  // ₱20 COINS
  // ===================================================

  while (remaining >= 20)
  {
    dispenseTwentyPeso();

    remaining -= 20;

    Serial.print(F("REMAINING CHANGE: PHP "));
    Serial.println(remaining);
  }


  // ===================================================
  // ₱10 COINS
  // ===================================================

  while (remaining >= 10)
  {
    dispenseTenPeso();

    remaining -= 10;

    Serial.print(F("REMAINING CHANGE: PHP "));
    Serial.println(remaining);
  }


  // ===================================================
  // ₱5 COINS
  // ===================================================

  while (remaining >= 5)
  {
    dispenseFivePeso();

    remaining -= 5;

    Serial.print(F("REMAINING CHANGE: PHP "));
    Serial.println(remaining);
  }


  // ===================================================
  // ₱1 COINS
  // ===================================================

  while (remaining >= 1)
  {
    dispenseOnePeso(1);

    remaining -= 1;

    Serial.print(F("REMAINING CHANGE: PHP "));
    Serial.println(remaining);
  }


  // ===================================================
  // COMPLETE
  // ===================================================

  Serial.println(F("======================================"));
  Serial.println(F("CHANGE DISPENSING COMPLETE"));
  Serial.println(F("======================================"));
}