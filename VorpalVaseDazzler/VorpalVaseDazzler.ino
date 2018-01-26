#include <FastLED.h>

// Copyright (C) 2018 Vorpal Robotics, LLC
// This program is licensed under the Creative Commons By-Attribution Non-Commercial 4.0 license.

// This is a driver program for the Vorpal Vase Dazzler project.
//
// Because this is open source you are free to source your own parts, however we do have
// convenient kits that do not require any soldering. Please support us by considering
// using our kits!  See:
// https://vorpal-robotics-store.myshopify.com/products/vase-dazzler-kit-no-soldering-required
//
// For complete instructions on 3D printing and building the dazzler see:
// http://tinyurl.com/VaseDazzler

// You need the FASTLED library to compile this code:
// http://fastled.io



// We're using pins all on one side of the Nano to make it easier to install in
// the base.

#define POTPIN      A0
#define POT5VPIN    A1
#define POTGNDPIN   A2
#define BUTTONGNDPIN A3
#define BUTTONPIN   A4
#define LED_PIN     A5
#define NUM_LEDS    12
#define NUM_MODES   10    // number of distinct light modes

#define MINBRIGHTNESS  5      // below this and some colors don't show up at all
#define MAXBRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 60


CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

#define BUTTON_UNPUSHED 0
#define BUTTON_PUSHED   1
#define BUTTON_LONGPUSH 2
#define BUTTON_TAP      3

#define BUTTON_LONGHOLD 1000
#define BUTTON_DEBOUNCE 50

int curBrightness = 50;
int curSpeed = 500;
int curLightMode = 0;
int buttonState = BUTTON_UNPUSHED;
int buttonSubstate = BUTTON_UNPUSHED;
long buttonLastChangeTime = 0;

void updateButtonState() {
  int pushed = 1-digitalRead(BUTTONPIN); // will be 1 if pushed, 0 if not pushed

  if (pushed == buttonState) {  // no change to push status or change time
      // but we may have changed longpush status.
      if (pushed == BUTTON_PUSHED) {
        if (millis()-buttonLastChangeTime > BUTTON_LONGHOLD) {
          buttonSubstate = BUTTON_LONGHOLD;
        } else {
          buttonSubstate = pushed;
        }
      }
      return;  // nothing else to do since nothing really changed
  }

  // if we get here, the push status does not equal the current button state

  // if it's a release and we made it past the debounce time, register a button tap
  if (pushed == BUTTON_UNPUSHED && (millis()-buttonLastChangeTime) > BUTTON_DEBOUNCE) {
    buttonSubstate = BUTTON_TAP;
  } else {
    buttonSubstate = pushed;
  }
  buttonState = pushed;
  buttonLastChangeTime = millis();
}

int readBrightness() {
  curBrightness = map(analogRead(POTPIN), 0, 1023, MINBRIGHTNESS, MAXBRIGHTNESS);
}

void setup() {
    delay( 1000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    readBrightness();
    FastLED.setBrightness(  curBrightness );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    // provide power to potentiometer
    pinMode(POT5VPIN, OUTPUT);
    pinMode(POTGNDPIN, OUTPUT);
    digitalWrite(POT5VPIN, HIGH);
    digitalWrite(POTGNDPIN, LOW);

    pinMode(BUTTONGNDPIN, OUTPUT);
    digitalWrite(BUTTONGNDPIN, LOW);

    pinMode(BUTTONPIN, INPUT_PULLUP);

    Serial.begin(9600);
}

void IndicateModeNumber(int mode) {
  // use the LEDs to indicate the mode number. For example, if 
  // mode is 1, then 1 led will light. If mode is 2, then two LEDs will light
  // etc. Gives user feedback about which mode they are in.

  mode = constrain(mode, 1, NUM_LEDS);
  for (int i = 0; i < mode; i++) {
    leds[i] = CRGB::White;
  }
  for (int i = mode; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

void loop()
{

    updateButtonState();
    if (buttonSubstate != BUTTON_LONGHOLD) {
      readBrightness();
      FastLED.setBrightness(  curBrightness );
    }
    if (buttonSubstate == BUTTON_TAP) {
      buttonSubstate = BUTTON_UNPUSHED; // clear this because we read the tap
      curLightMode++;
      if (curLightMode > NUM_MODES) {
        curLightMode = 0;
      }
      IndicateModeNumber(curLightMode);
      FastLED.show();
      FastLED.delay(500);
    }
    Serial.print("M="); Serial.print(curLightMode); Serial.print(" B="); Serial.println(curBrightness);
    ChangePalette(curLightMode);
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}







// This function sets up a palette of pink, red, and purple
void SetupPinkRedPurplePalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB red  = CHSV( HUE_RED, 255, 255);
    CRGB pink  = CHSV( HUE_PINK, 255, 255);
    
    currentPalette = CRGBPalette16(
                                   pink,  pink,  pink,  pink,
                                   pink, red, red,  red,
                                   red,  red,  purple,  purple,
                                   purple, purple, purple,  purple );
}

CRGB FlameFlicker[16];

void SetupFlameFlickers()
{

    CRGB flame1 = CHSV( HUE_YELLOW, 255, 255);
    CRGB flame2  = CHSV( HUE_YELLOW, 220, 220);
    CRGB flame3  = CHSV( HUE_YELLOW, 200, 180);
    CRGB flame4  = CHSV( HUE_YELLOW, 150, 140);

      if (random(100) < 2) {
        switch (random(5)) {
          case 0: 
              for (int i = 0; i < 8; i++) {
                FlameFlicker[i] = flame1;
                FlameFlicker[i+8] = flame2;
              }
              break;
          case 1: 
              for (int i = 0; i < 8; i++) {
                FlameFlicker[i] = flame2;
                FlameFlicker[i+8] = flame1;
              }
              break;
          case 2: 
              for (int i = 0; i < 8; i++) {
                FlameFlicker[i] = flame3;
                FlameFlicker[i+8] = flame3;
              }
              break;
          case 3: // for dark it's better for the black to be very brief
              for (int i = 0; i < 13; i++) {
                FlameFlicker[i] = flame2;
              }
              for (int i = 13; i < 16; i++) {
                FlameFlicker[i] = flame4;
              }
              break;
          case 4:
              for (int i = 0; i < 8; i++) {
                FlameFlicker[i] = flame4;
                FlameFlicker[i+8] = flame1;
              }
              break;
        }
      }
    
    currentPalette = CRGBPalette16(
                                   FlameFlicker[0],
                                   FlameFlicker[1],
                                   FlameFlicker[2],
                                   FlameFlicker[3],
                                   FlameFlicker[4],
                                   FlameFlicker[5],
                                   FlameFlicker[6],
                                   FlameFlicker[7],
                                   FlameFlicker[8],
                                   FlameFlicker[9],
                                   FlameFlicker[10],
                                   FlameFlicker[11],
                                   FlameFlicker[12],
                                   FlameFlicker[13],
                                   FlameFlicker[14],
                                   FlameFlicker[15]
                                   
                     );
}

void ChangePalette(int curmode)
{


    if (curmode == 0) {  // choose one of the other modes based on time
      uint8_t secondHand = (millis() / 1000) % 60;
      static uint8_t lastSecond = 99;
      
      if( lastSecond != secondHand) {
          lastSecond = secondHand;
          int chg = 1 + lastSecond/6;
          
          ChangePalette(chg);
          Serial.print("CHG="); Serial.println(chg);
      }
    } else {
      currentBlending = LINEARBLEND;
      switch (curmode) {
        case 1: currentPalette = RainbowColors_p;       break;
        case 2: currentPalette = LavaColors_p;          break;
        case 3: currentPalette = RainbowStripeColors_p; break;
        case 4: SetupPinkRedPurplePalette();           break;
        case 5: currentPalette = OceanColors_p;         break;
        case 6: currentPalette = CloudColors_p;         break;
        case 7: currentPalette = PartyColors_p;         break;
        case 8: SetupFlameFlickers();                   break;
        case 9: currentPalette = ForestColors_p;        break; 
        case 10: 
        default:
                currentPalette = HeatColors_p;          break;
      }
    }
}

