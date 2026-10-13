// =====================================================
// SNAP AND PRINT
// BILL ACCEPTOR
// =====================================================


// =====================================================
// BILL IR SENSOR
// =====================================================

const byte BILL_IR_PIN = 3;


// =====================================================
// BILL STATE
// =====================================================

bool billPresent = false;


// =====================================================
// SENSOR STATE
// =====================================================

byte billIRLastState = HIGH;


// =====================================================
// INITIALIZE
// =====================================================

void setupBillAcceptor()
{
  pinMode(BILL_IR_PIN, INPUT);

  billIRLastState = digitalRead(BILL_IR_PIN);

  billPresent = false;
}


// =====================================================
// UPDATE BILL ACCEPTOR
// =====================================================

void updateBillAcceptor()
{
  byte state = digitalRead(BILL_IR_PIN);


  // Nothing changed
  if (state == billIRLastState)
    return;


  billIRLastState = state;


  // ===================================================
  // BILL PRESENT
  // ===================================================

  if (state == LOW)
  {
    billPresent = true;


    // Only report it during an active payment.

    if (paymentActive)
    {
      Serial.println(F("BILL:PRESENT"));

      Serial.println(
        F("STATUS:BILL DETECTED - DENOMINATION UNKNOWN")
      );
    }

    return;
  }


  // ===================================================
  // BILL REMOVED
  // ===================================================

  if (state == HIGH)
  {
    if (billPresent)
    {
      billPresent = false;

      if (paymentActive)
      {
        Serial.println(F("BILL:REMOVED"));
      }
    }
  }
}