#include <Adafruit_NeoPixel.h>
#include "RP2040_PWM.h" 

#define NUMPIXELS         1

#define PIN_NOOD_TOP      4
#define PIN_NOOD_BOTTOM   5
#define PIN_NOOD_INSIDE   6
#define PWM_FREQ          50 * 1000     // "50kHz ought to be good enough for anyone"

#define LED_RED           0xFF0000
#define LED_GREEN         0x00FF00
#define LED_BLUE          0x0000FF
#define LED_YELLOW        0xFFFF00
#define LED_PURPLE        0xFF00FF
#define LED_WHITE         0xFFFFFF

enum NOOD {
  NOOD_TOP    = 0x01,
  NOOD_BOTTOM = 0x02,
  NOOD_INSIDE = 0x04
};

Adafruit_NeoPixel indicator(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
RP2040_PWM* noodTop;
RP2040_PWM* noodBottom;
RP2040_PWM* noodInside;

bool enableNoodTop = true;
bool enableNoodBottom = true;
bool enableNoodInside = true;

void setup() {
  Serial.begin(115200);
  indicator.begin(); 
  indicator.setBrightness(20); 
  indicator.fill(LED_RED);
  indicator.show();

  noodTop    = new RP2040_PWM(PIN_NOOD_TOP,    PWM_FREQ, 0);
  noodBottom = new RP2040_PWM(PIN_NOOD_BOTTOM, PWM_FREQ, 0);
  noodInside = new RP2040_PWM(PIN_NOOD_INSIDE, PWM_FREQ, 0);

  noodTop->setPWM();
  noodBottom->setPWM();
  noodInside->setPWM();
}

void loop() {
  A_AllOff();

  A_PowerOn(1 * 1000, 1 * 1000, 2 * 1000);
    delay(2 * 1000);
  A_OnlyInside();
  A_LaserShots(6, 300, 50, 100);
    delay(2 * 1000);
  A_Alternating(500, 10);
  A_AllOff();
    delay(2 * 1000);
  A_PowerOnReverse(1 * 1000, 1 * 1000, 2 * 1000);
    delay(2 * 1000);
  A_Pulse(5, 100, 0, 1 * 1000, 1 * 1000, 0, 0);
    delay(2 * 1000);
  A_Cycle(5, 500);
  A_AllOn();
  A_Pulse(5, 100, 50, 1 * 1000, 1 * 1000, 0, 0);
    delay(2 * 1000);
  A_LaserShots(9, 700, 200, 200);
    delay(2 * 1000);
  A_Cycle(20, 100);
    delay(2 * 1000);
  A_Shutdown(2 * 1000);

  A_AllOff();
    delay(2 * 1000);
}

////////////////
// HELPERS
////////////////
void setNood(int noodBitMask, int dutyCycle) {
  if(noodBitMask & NOOD_TOP && enableNoodTop) noodTop->setPWM(PIN_NOOD_TOP, PWM_FREQ, dutyCycle);
  if(noodBitMask & NOOD_BOTTOM && enableNoodBottom) noodBottom->setPWM(PIN_NOOD_BOTTOM, PWM_FREQ, dutyCycle);
  if(noodBitMask & NOOD_INSIDE && enableNoodInside) noodInside->setPWM(PIN_NOOD_INSIDE, PWM_FREQ, dutyCycle);
}

// timeMS must be 100 or greater
void fadeIn(int noodBitMask, long timeMS, int startBrightness = 0, int maxBrightness = 100) {
  if(timeMS < 100) return;
  for(int i = startBrightness; i <= maxBrightness; i++) {
    setNood(noodBitMask, i);
    delay(timeMS / 100);
  }
}

// timeMS must be 100 or greater
void fadeOut(int noodBitMask, long timeMS, int startBrightness = 100, int minBrightness = 0) {
  if(timeMS < 100) return;
  for(int i = startBrightness; i >= minBrightness; i--) {
    setNood(noodBitMask, i);
    delay(timeMS / 100);
  }
}

////////////////
// ANIMATIONS
////////////////
void A_AllOff() {
  setNood(NOOD_TOP + NOOD_BOTTOM + NOOD_INSIDE, 0);
}

void A_AllOn() {
  setNood(NOOD_TOP + NOOD_BOTTOM + NOOD_INSIDE, 100);
}

void A_PowerOn(int outsideOnSpeed, int insideOnSpeed, int delayTime) {
  fadeIn(NOOD_TOP + NOOD_BOTTOM, outsideOnSpeed);
  delay(delayTime);
  fadeIn(NOOD_INSIDE, insideOnSpeed);
}

void A_PowerOnReverse(int outsideOnSpeed, int insideOnSpeed, int delayTime) {
  fadeIn(NOOD_INSIDE, insideOnSpeed);
  delay(delayTime);
  fadeIn(NOOD_TOP + NOOD_BOTTOM, outsideOnSpeed);
}

void A_OnlyInside() {
  setNood(NOOD_TOP + NOOD_BOTTOM, 0);
  setNood(NOOD_INSIDE, 100);
}

void A_LaserShots(int numShots, int fadeSpeed, int onDelay, int offDelay) {
  for(int i = 0; i < numShots; i++) {
    setNood(NOOD_TOP + NOOD_BOTTOM, 100);
    delay(onDelay);
    fadeOut(NOOD_TOP + NOOD_BOTTOM, fadeSpeed);
    delay(offDelay);
  }
}

void A_Alternating(int startTime, int finalRepeat) {
  for(int i = startTime; i >= 50; i-=50) {
    setNood(NOOD_TOP, 100);
    setNood(NOOD_BOTTOM, 0);
    delay(i);
    setNood(NOOD_BOTTOM, 100);
    setNood(NOOD_TOP, 0);
    delay(i);
  }

  for(int i = 0; i <= finalRepeat; i++) {
    setNood(NOOD_TOP, 100);
    setNood(NOOD_BOTTOM, 0);
    delay(50);
    setNood(NOOD_BOTTOM, 100);
    setNood(NOOD_TOP, 0);
    delay(50);
  }
}

void A_Pulse(int numPulses, int maxBrightness, int minBrightness, int fadeInTime, int fadeOutTime, int onDelay, int offDelay) {
  setNood(NOOD_TOP + NOOD_BOTTOM, 0);
  for(int i = 0; i < numPulses; i++) {
    fadeIn(NOOD_TOP + NOOD_BOTTOM, fadeInTime, minBrightness, maxBrightness);
    delay(onDelay);
    fadeOut(NOOD_TOP + NOOD_BOTTOM, fadeOutTime, maxBrightness, minBrightness);
    delay(offDelay);
  }
}

void A_Cycle(int numCycles, int delayTime) {
  A_AllOff();
  for(int i = 0; i < numCycles; i++) {
    setNood(NOOD_TOP, 100);
    setNood(NOOD_BOTTOM + NOOD_INSIDE, 0);
    delay(delayTime);
    setNood(NOOD_BOTTOM, 100);
    setNood(NOOD_TOP + NOOD_INSIDE, 0);
    delay(delayTime);
    setNood(NOOD_INSIDE, 100);
    setNood(NOOD_TOP + NOOD_BOTTOM, 0);
    delay(delayTime);
  }
}

void A_Shutdown(int fadeTime) {
  fadeOut(NOOD_INSIDE, fadeTime);
}
