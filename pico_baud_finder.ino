#include "MedianFilter.h"

#define USE_INTERUPT
#define RATE_SAMPLES 20
#define NUM_SAMPLES 50
#define INITIAL_BAUD 9600
#define USB_BAUD 230400
#define RX_PIN 1




long baudRate = INITIAL_BAUD;
long serial1Baud = baudRate;

long prevRate = 0;

bool debug = false;
bool lockBaud = false;

#ifdef USE_INTERUPT
MedianFilter pulses(200, INITIAL_BAUD);
#endif

void setup() {
  pinMode(RX_PIN, INPUT_PULLUP);  // make sure serial in is a input pin
  //digitalWrite(RX_PIN, HIGH);  // pull up enabled just for noise protection
#ifdef USE_INTERUPT
  delay(1000);
  // Start measurement each time PIN goes LOW
  attachInterrupt(digitalPinToInterrupt(RX_PIN), measurePulse, LOW);
#endif
// }

// void setup1() {
  Serial.begin(USB_BAUD);
  delay(100);
  Serial1.setFIFOSize(128);
  Serial1.begin(baudRate);
  delay(500);
}

void switchBaud() {
  if (baudRate == -1) {
    Serial.println("");
    Serial.println("");
    Serial.println(">>>>>> UNABLE TO DETECT BAUD <<<<<<<");
    Serial.print(">>>>>> APPROX BAUD: ");
    Serial.print(approximateBaud(prevRate));
    Serial.println("<<<<<<<");
    Serial.println("");
    Serial.println("");
  // } else if (serial1Baud != baudRate) {
  } else {
    Serial.println("");
    Serial.println("");
    Serial.print(">>>>>>>>>>>>> Switching baud rate from ");
    Serial.print(serial1Baud);
    Serial.print(" to ");
    Serial.println(baudRate);
    Serial.println("");
    Serial.println("");
    serial1Baud = baudRate;
    Serial1.end();
    Serial.end();
    Serial.begin();
    Serial1.begin(serial1Baud);
  }
}

// void loop1() {
//   if (lockBaud) {
//     // Redirect the outputs
//     redirectSerialToSerial(&Serial1, &Serial);
//     redirectSerialToSerial(&Serial, &Serial1);
//     //delay(10);
//   }
// }

long lastBounce = 5000;
long debounceTime = 1000;
void loop() {

  if (millis() < 5000) {
    Serial.print("Waiting for data to be sampled...");
    Serial.println(millis());
    return;
  }

  // Enable baud locking, simply exit loop early if lockBaud is true
  if (BOOTSEL && (millis() - lastBounce > debounceTime)) {
    lastBounce = millis();
    Serial.println();
    if (!lockBaud) {
      // Start measurement each time PIN goes LOW
      detachInterrupt(digitalPinToInterrupt(RX_PIN));
      switchBaud();
      Serial.println(">>>>>>>>>>>> Switching to terminal mode");
    } else {

      // Start measurement each time PIN goes LOW
      attachInterrupt(digitalPinToInterrupt(RX_PIN), measurePulse, LOW);
      Serial.println(">>>>>>>>>>>> Switching to detect mode");
    }
    Serial.println();
    lockBaud = !lockBaud;
    digitalWrite(LED_BUILTIN, lockBaud);
  }

  if (lockBaud) { 
    if (Serial1.available() > 0) Serial.write(Serial1.read());
    if (Serial.available() > 0) Serial1.write(Serial.read());
    //redirectSerialToSerial(&Serial1, &Serial);
    //redirectSerialToSerial(&Serial, &Serial1);
    return; 
    }


#ifdef USE_INTERUPT
  long rate = pulses.getMin();

  // Check if the current rate is the same as last, if it has not changed exit loop
  if (rate == prevRate) return;
  prevRate = rate;

  // Calculate the likely baudRate
  if (rate > 0) { baudRate = calculateBaud(rate); }
#else
  MedianFilter rates(RATE_SAMPLES, 0);
  for (int i = 0; i < RATE_SAMPLES; i++) {
    long value = detRate(RX_PIN);
    rates.in(value);
  }

  long rate = rates.getMin();
  baudRate = calculateBaud(rate);
#endif

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

long approximateBaud(long rate) {
  return 1 / (rate * 0.000001);
}


void redirectSerialToSerial(Stream *input, Stream *output) {
  if (input->available() > 0)
    output->write(input->read());
}

void redirectSerialToOutput1AndOutput2(Stream *input, Stream *output1, Stream *output2) {
  if (input->available() > 0) {
    int byte = input->read();
    output1->write(byte);
    output2->write(byte);
  }
}


// KNOWN BAUDS
// | Baud   | Rate Range  |
// | 300    | 3333        |
// | 600    | 1666-1667   |
// | 1200   | 832-834     |
// | 2400   | 415-417     |
// | 3600   | 274-278     |
// | 4800   | 206-209     |
// | 7200   | 138-139     |
// | 9600   | 103-104     |
// | 14400  | 65-70       |
// | 19200  | 50-52       |
// | 28800  |             |
// | 38400  | 25-26       |
// | 57600  | 16-17       |
// | 115200 | 6-9         |
// | 230400 | 3-5         |
// | 460800 | Unstable    |

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


#ifdef USE_INTERUPT
void measurePulse() {
  pulses.in(pulseIn(RX_PIN, LOW));
}
#else
long detRate(int recpin) {
  MedianFilter values(NUM_SAMPLES, 0);

  for (int i = 0; i < NUM_SAMPLES; i++) {
    while (digitalRead(recpin) == HIGH) {}  // wait for low bit to start
    long value = pulseIn(recpin, LOW);
    if (value > 0) values.in(value);
  }

  return values.out();
}
#endif