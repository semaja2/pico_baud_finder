/*

Project Name: pico baud finder
Project Author: semaja2

*/


#include "MedianFilter.h"

// Set initial baud for terminal
#define INITIAL_BAUD 9600

// Set USB Serial Baud rate, not needed on RP2040
#define USB_BAUD 230400

// Set the UART RX pin to run detect mode, should match "Serial1" pin
#define RX_PIN 1

long baudRate = INITIAL_BAUD;
long prevRate = 0;
bool terminalMode = false;
long lastBounce = WARMUP;

MedianFilter pulses(200, INITIAL_BAUD);

void setup() {
  Serial.begin(USB_BAUD);
  delay(100);
  switchToDetectMode();
  Serial1.setFIFOSize(1024); // Increase FIFO buffer
  Serial1.begin(baudRate);
  delay(500);
}

void loop() {

  // Enable baud locking, simply exit loop early if terminalMode is true
  if (BOOTSEL && (millis() - lastBounce > 1000)) {
    lastBounce = millis();
    if (!terminalMode) {
      switchToTerminalMode();
    } else {
      switchToDetectMode();
    }
    digitalWrite(LED_BUILTIN, terminalMode);
  }

  if (terminalMode) {
    if (Serial1.available() > 0) Serial.write(Serial1.read());
    if (Serial.available() > 0) Serial1.write(Serial.read());
    return;
  } else {
    long rate = pulses.getMin();

    // Check if rate has changed, if it has recalculate
    if (rate != prevRate) {
      prevRate = rate;

      // Calculate the likely baudRate
      if (rate > 0) { baudRate = calculateBaud(rate); }

      Serial.print("============================ ");
      Serial.print(millis());
      Serial.println(" ============================");
      Serial.print("Calculated_Baud:");
      Serial.println(calculateBaud(rate));
      Serial.print("Rate:");
      Serial.println(rate);
      Serial.print("Approximate_Baud:");
      Serial.println(approximateBaud(rate));
    }
  }
}


void switchBaud() {
  if (baudRate == -1) {
    Serial.println("");
    Serial.println("");
    Serial.println(">>>>>> UNABLE TO DETECT BAUD <<<<<<<");
    Serial.print(">>>>>> APPROX BAUD: ");
    Serial.print(approximateBaud(prevRate));
    Serial.println("<<<<<<<");
    Serial.println("Returning to detect mode");
    Serial.println("");
    Serial.println("");
  } else {
    Serial.println("");
    Serial.println("");
    Serial.print("Setting up terminal mode with baud ");
    Serial.println(baudRate);
    Serial.println("");
    Serial.println("");

    //Reset serial interfaces and setup new baud
    Serial1.end();
    Serial.end();
    Serial.begin();
    Serial1.begin(baudRate);
  }
}

void switchToDetectMode() {
  terminalMode = false;
  pinMode(RX_PIN, INPUT_PULLUP);  // make sure serial in is a input pin
  // Start measurement each time PIN goes LOW
  attachInterrupt(digitalPinToInterrupt(RX_PIN), measurePulse, LOW);
  Serial.println(">>>>>>>>>>>> Switching to detect mode");
}

void switchToTerminalMode() {
  // Start measurement each time PIN goes LOW
  detachInterrupt(digitalPinToInterrupt(RX_PIN));
  switchBaud();
  Serial.println(">>>>>>>>>>>> Switching to terminal mode");
  terminalMode = true;
}

long approximateBaud(long rate) {
  return 1 / (rate * 0.000001);
}


// Returns -1 if it cant find a result
long calculateBaud(long rate) {
  if (rate >= 3330 && rate < 3336) return 300;
  else if (rate >= 1660 && rate < 1670) return 600;
  else if (rate >= 825 && rate < 840) return 1200;
  else if (rate >= 410 && rate < 420) return 2400;
  else if (rate >= 270 && rate < 280) return 3600;
  else if (rate >= 200 && rate < 215) return 4800;
  else if (rate >= 130 && rate < 145) return 7200;
  else if (rate >= 95 && rate < 110) return 9600;
  else if (rate >= 60 && rate < 75) return 14400;
  else if (rate >= 45 && rate < 55) return 19200;
  else if (rate >= 29 && rate < 40) return 28800;
  else if (rate >= 20 && rate < 29) return 38400;
  else if (rate >= 10 && rate < 20) return 57600;
  else if (rate >= 3 && rate < 10) return 115200;
  //else if (rate >= 1 && rate < 3) return 230400;
  else return -1;
}


void measurePulse() {
  pulses.in(pulseIn(RX_PIN, LOW));
}