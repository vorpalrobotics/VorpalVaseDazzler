#include <FastLED.h>

// Copyright (C) 2018 Vorpal Robotics, LLC
// This program is licensed under the Creative Commons Attribution-ShareAlike 4.0 International license.
//
// A convenient electronics kit for this project is available at http://store.vorpalrobotics.com
//
// This is a driver program for the Vorpal Vase Dazzler project.
// A button cycles between light patterns, and a potentiometer selects brightness of a 12 Addressable RGB LED Ring.
//
// The project is meant to sit in a base that a 3D printed semi-transparent vase sits on top of. It's better to
// print the vase with no bottom layers, or very minimal bottom layers, to let the light shine through.
// Transparent or semi-transparent vase materials are of course best.
// For more details see our wiki: http://www.vorpalrobotics.com/wiki/Vorpal_Vase_Dazzler

// We're using pins mostly on just one side of the Nano to make it easier to install in
// the base.

#define POTPIN      A0
#define POT5VPIN    A1
#define POTGNDPIN   A2
#define BUTTONGNDPIN A3
#define BUTTONPIN   A4
#define LED_PIN     A5
#define NUM_LEDS    12

#define MINBRIGHTNESS  10
#define MAXBRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100


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
}


void loop()
{
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


// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

CRGB FlameFlicker[16];

void SetupFlameFlickers()
{
    CRGB red = CHSV( HUE_RED, 255, 255);
    CRGB yellow  = CHSV( HUE_YELLOW, 255, 255);
    CRGB black  = CRGB::Black;
    CRGB white = CRGB::White;

    for (int i = 0; i < 16; i++) {
      if (random(100) < 5) {
        switch (random(6)) {
          case 0: FlameFlicker[i] = red; break;
          case 1: FlameFlicker[i] = yellow; break;
          case 2: FlameFlicker[i] = black; break;
          case 3: FlameFlicker[i] = white; break;
          case 4:
          case 5: FlameFlicker[i] = FlameFlicker[(i+1)%16]; break;
        }
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


// select a color palette

void ChangePalette(int curmode)
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if (curmode == 0) {  // choose one of the other modes based on time
    
      if( lastSecond != secondHand) {
          lastSecond = secondHand;
          ChangePalette(1+lastSecond/6);
      }
    } else {
      currentBlending = LINEARBLEND;
      switch (curmode) {
        case 1: currentPalette = RainbowColors_p;       break;
        case 2: currentPalette = LavaColors_p;          break;
        case 3: currentPalette = RainbowStripeColors_p; break;
        case 4: SetupPurpleAndGreenPalette();           break;
        case 5: currentPalette = OceanColors_p;         break;
        case 6: currentPalette = CloudColors_p;         break;
        case 7: currentPalette = PartyColors_p;         break;
        case 8: SetupFlameFlickers();                   break;
        case 9:
        
        case 10: default:
          break;
      }
  }
}



