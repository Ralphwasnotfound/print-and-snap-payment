// =====================================================
// SNAP AND PRINT
// COIN ACCEPTOR
// =====================================================


// =====================================================
// PIN
// =====================================================

const byte COIN_ACCEPTOR_PIN = 2;


// =====================================================
// PULSE SETTINGS
// =====================================================

// Time with NO pulse before we consider
// the current coin completely finished.
const unsigned long COIN_PULSE_TIMEOUT = 500;


// =====================================================
// COIN STATE
// =====================================================

volatile unsigned int coinPulseCount = 0;

volatile unsigned long lastCoinPulseTime = 0;

volatile bool coinPulseActive = false;


// =====================================================
// INITIALIZE
// =====================================================

void setupCoinAcceptor()
{
  pinMode(COIN_ACCEPTOR_PIN, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(COIN_ACCEPTOR_PIN),
    coinPulseISR,
    FALLING
  );
}


// =====================================================
// INTERRUPT
// =====================================================

void coinPulseISR()
{
  if (!paymentActive)
    return;

  coinPulseCount++;

  lastCoinPulseTime = millis();

  coinPulseActive = true;
}


// =====================================================
// UPDATE COIN ACCEPTOR
// =====================================================

void updateCoinAcceptor()
{
  if (!coinPulseActive)
    return;


  // ===================================================
  // WAIT FOR COMPLETE PULSE TRAIN
  // ===================================================

  noInterrupts();

  unsigned long lastPulse = lastCoinPulseTime;

  interrupts();


  if (millis() - lastPulse < COIN_PULSE_TIMEOUT)
  {
    return;
  }


  // ===================================================
  // COPY AND CLEAR PULSE DATA
  // ===================================================

  noInterrupts();

  unsigned int pulses = coinPulseCount;

  coinPulseCount = 0;
  coinPulseActive = false;

  interrupts();


  // ===================================================
  // DEBUG
  // ===================================================

  Serial.print(F("PULSE COUNT: "));
  Serial.println(pulses);


  // ===================================================
  // DETERMINE DENOMINATION
  // ===================================================

  int coinValue = 0;


  if (pulses == 1)
  {
    coinValue = 1;
  }
  else if (pulses == 5)
  {
    coinValue = 5;
  }
  else if (pulses == 10)
  {
    coinValue = 10;
  }
  else if (pulses == 20)
  {
    coinValue = 20;
  }


  // ===================================================
  // INVALID PULSE COUNT
  // ===================================================

  if (coinValue == 0)
  {
    Serial.print(F("STATUS:INVALID COIN PULSES "));
    Serial.println(pulses);

    return;
  }


  // ===================================================
  // VALID COIN
  // ===================================================

  Serial.print(F("COIN:"));
  Serial.println(coinValue);


  // ===================================================
  // SEND TO PAYMENT BRAIN
  // ===================================================

  addPayment(coinValue);
}


// =====================================================
// CLEAR COIN PULSES
// =====================================================

void clearCoinPulses()
{
  noInterrupts();

  coinPulseCount = 0;

  coinPulseActive = false;

  interrupts();
}