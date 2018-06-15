// MIDI tx
// lcd pins 4, 5, 6, 7, 8, 9
// rotary enc pins 2, 3, A7
// switch inputs A0, A1, A2, A3, A4, A5
// hc595 driven leds 10, 11, 12
#include <MIDI.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "DualFunctionButton.h"

const bool DEBUG = 0;

//Variables for inputs
const uint8_t inputs = 6;
const uint8_t inputPin[inputs] = {A0, A1, A2, A3, A4, A5};
bool buttonState[inputs] = {1, 1, 1, 1, 1, 1};
bool playing[inputs] = {false, false, false, false, false, false};  //Is note currently playing
unsigned long lasttrig[inputs];
uint8_t debounce = 10;

// Variables for rotary encoder
const uint8_t encClk = 2; // Needs Interupt pin
const uint8_t encDt = 3;
const uint8_t encSw = 4;

volatile boolean up = false;
volatile boolean down = false;
volatile boolean middle = false;
// int selectButtonState = 0;
// int lastSelectButtonState = 0;
DualFunctionButton button(encSw, 1000, INPUT_PULLUP);

uint8_t lastCount = 0;
// volatile uint8_t virtualPosition = 0;
bool swState = true;

// Pins for hc595 to drive LEDs
const uint8_t ledClockPin = 10;
const uint8_t ledLatchPin = 11;
const uint8_t ledDataPin = 12;
byte ledBits = B00000000;

// Note names matched to MIDI value
// uint8_t midiChannel = 1;
uint8_t midiAddress = inputs; //use address space after inputs
const uint8_t numNotes = 84;
const uint8_t noteOffset = 12;
char noteName[numNotes][5] = { // Add 12 to index to get MIDI note
  "C-1", "C#-1", "D-1", "Eb-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "Bb-1", "B-1",
  "C0", "C#0", "D0", "Eb0", "E0", "F0", "F#0", "G0", "G#0", "A0", "Bb0", "B0",
  "C1", "C#1", "D1", "Eb1", "E1", "F1", "F#1", "G1", "G#1", "A1", "Bb1", "B1",
  "C2", "C#2", "D2", "Eb2", "E2", "F2", "F#2", "G2", "G#2", "A2", "Bb2", "B2",
  "C3", "C#3", "D3", "Eb3", "E3", "F3", "F#3", "G3", "G#3", "A3", "Bb3", "B3",
  "C4", "C#4", "D4", "Eb4", "E4", "F4", "F#4", "G4", "G#4", "A4", "Bb4", "B4",
  "C5", "C#5", "D5", "Eb5", "E5", "F5", "F#5", "G5", "G#5", "A5", "Bb5", "B5"
};

// temp value for EEPROM reading
uint8_t value;

// default MIDI notes - replaced from EEPROM
uint8_t notes[inputs] = {60, 61, 13, 65, 67, 69};
uint8_t midiChannels[inputs] = {1, 1, 1, 1, 1, 1};

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

// Initialise display
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 7, 6, 5); // pin CS (labelled CE) tied to GND
bool backlight = true;
uint8_t contrast = 60;
const uint8_t backlightPin = 9;

// Menu
uint8_t page = 1;
uint8_t menuitem = 0;

static const unsigned char PROGMEM abLogo[] = {
0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0xFF,
0xE0, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xF8, 0x00, 0x00, 0x00,
0x7F, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0xFF, 0xF8, 0xFE, 0x00, 0x00, 0x00, 0xFD, 0xF8, 0x7E, 0x00,
0x00, 0x01, 0xF9, 0xF8, 0x3F, 0x00, 0x00, 0x03, 0xF9, 0xF8, 0x1F, 0x00, 0x00, 0x03, 0xF1, 0xF8,
0x1F, 0x00, 0x00, 0x07, 0xE1, 0xF8, 0x1F, 0x00, 0x00, 0x07, 0xE1, 0xF8, 0x1F, 0x00, 0x00, 0x0F,
0xC1, 0xF8, 0x1F, 0x00, 0x00, 0x0F, 0x81, 0xF8, 0x1F, 0x00, 0x00, 0x1F, 0x81, 0xF8, 0x1F, 0x00,
0x00, 0x1F, 0x01, 0xF8, 0x1F, 0x00, 0x00, 0x3F, 0x01, 0xF8, 0x3F, 0x00, 0x00, 0x3E, 0x01, 0xF8,
0x7E, 0x00, 0x00, 0x7E, 0x01, 0xF9, 0xFE, 0x00, 0x00, 0x7C, 0x01, 0xFF, 0xFC, 0x00, 0x00, 0xFF,
0xFF, 0xFF, 0xF8, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x03, 0xF0, 0x01, 0xFF,
0xFF, 0xE0, 0x03, 0xE0, 0x01, 0xF8, 0x3F, 0xF0, 0x03, 0xE0, 0x01, 0xF8, 0x07, 0xF8, 0x07, 0xE0,
0x01, 0xF8, 0x03, 0xF8, 0x07, 0xC0, 0x01, 0xF8, 0x00, 0xF8, 0x07, 0xC0, 0x01, 0xF8, 0x00, 0xFC,
0x0F, 0xC0, 0x01, 0xF8, 0x00, 0x7C, 0x0F, 0x80, 0x01, 0xF8, 0x00, 0x7C, 0x0F, 0x80, 0x01, 0xF8,
0x00, 0x7C, 0x1F, 0x80, 0x01, 0xF8, 0x00, 0x7C, 0x1F, 0x00, 0x01, 0xF8, 0x00, 0x7C, 0x1F, 0x00,
0x01, 0xF8, 0x00, 0x7C, 0x1F, 0x00, 0x01, 0xF8, 0x00, 0xFC, 0x3F, 0x00, 0x01, 0xF8, 0x00, 0xFC,
0x3E, 0x00, 0x01, 0xF8, 0x01, 0xF8, 0x3E, 0x00, 0x01, 0xF8, 0x07, 0xF8, 0x7E, 0x00, 0x01, 0xF8,
0x3F, 0xF0, 0x7C, 0x00, 0x01, 0xFF, 0xFF, 0xE0, 0x7C, 0x00, 0x01, 0xFF, 0xFF, 0xC0, 0x7C, 0x00,
0x01, 0xFF, 0xFF, 0x00, 0xFC, 0x00, 0x01, 0xFF, 0xFE, 0x00, 0xF8, 0x00, 0x01, 0xFF, 0xE0, 0x00
};

void setup() {
  // hc595 I/Os
  pinMode(ledClockPin, OUTPUT);
  pinMode(ledLatchPin, OUTPUT);
  pinMode(ledDataPin, OUTPUT);
  // reset leds ASAP
  digitalWrite(ledLatchPin, LOW);  // prepare shift register for data
  shiftOut(ledDataPin, ledClockPin, LSBFIRST, B00000000); // send data
  digitalWrite(ledLatchPin, HIGH); // update display

  MIDI.begin();
  if (DEBUG) {
    Serial.begin(115200);
  }
  // Switch inputs with pullups
  for (uint8_t i = 0; i < inputs; i++) {
    pinMode (inputPin[i], INPUT_PULLUP);
  }

  // Set last trigger time for each input
  for (uint8_t i = 0; i < inputs; i++) {
    lasttrig[i] = millis();
  }

  //read EEPROM values
  for (uint8_t i = 0; i < inputs; i++) {
    value = EEPROM.read(i);
    if (value > 11 && value < 128) {
      notes[i] = value;
    }
  }
  for (uint8_t i = 0; i < inputs; i++) {
    value = EEPROM.read(midiAddress + i);
    if (value >= 1 && value <=16) { //valid midiChannel
      midiChannels[i] = value;
    }
  }

  // rotary encoder I/Os
  pinMode(encClk, INPUT);
  pinMode(encDt, INPUT);
  pinMode(encSw, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encClk), isr, LOW);

  // display
  pinMode(backlightPin, OUTPUT);
  turnBacklightOn();
  
  display.begin();
  display.clearDisplay();
  display.setContrast(contrast);

  display.drawBitmap(19, 0, abLogo, 48, 48, BLACK);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();

//debug info to Serial
  if (DEBUG) {
    Serial.println ("Notes:");
    for (uint8_t i = 0; i< inputs; i++) {
      Serial.print(notes[i]);
      Serial.print("\t");
    }
    Serial.println();
    Serial.println("MIDI channels:");
    for (uint8_t i = 0; i< inputs; i++) {
      Serial.print(midiChannels[i]);
      Serial.print("\t");
    }
    Serial.println();
  }
  updateDisplay();

}

void loop () {
  updateDisplay();

  // selectButtonState = digitalRead(encSw);
  // checkIfSelectButtonIsPressed();

  if (up && page == 1) {
    up = false;
    if (menuitem < inputs-1) {
      menuitem++;
    }
  } else if (up && page == 2) {
    up = false;
    if (notes[menuitem] < 95) {
      notes[menuitem]++;
    }
  } else if (up && page == 3) {
    up = false;
    if (midiChannels[menuitem] < 16) {
      midiChannels[menuitem]++;
      updateDisplay();
    }
  }

  if (down && page == 1 && menuitem >= 0) {
    down = false;
    if (menuitem > 0) {
      menuitem--;
    }
  } else if (down && page == 2) {
    down = false;
    if (notes[menuitem] > 12) {
      notes[menuitem]--;
    }
  } else if (down && page == 3) {
    down = false;
    if (midiChannels[menuitem] > 1) {
      midiChannels[menuitem]--;
      updateDisplay();
    }
  }

  if (button.shortPress()) {
    if (page == 1 ) {
      page=2;
    } else if (page ==2) {
      page = 1;
      //write value to EEPROM
      if (DEBUG) {
        Serial.print("Writing value ");
        Serial.print(notes[menuitem]);
        Serial.print(" to ");
        Serial.println(menuitem);
      }
      EEPROM.update(menuitem, notes[menuitem]);
    } else if (page ==3) {
      page = 1;
      //write value to EEPROM
      if (DEBUG) {
        Serial.print("Writing MIDI channel value ");
        Serial.print(midiChannels[menuitem]);
        Serial.print(" to ");
        Serial.println(menuitem);
      }
      EEPROM.update(midiAddress + menuitem, midiChannels[menuitem]);
    }
  }
  if (button.longPress() && page == 2) {
    if (DEBUG) {
      Serial.println("Long Press");
    }
    page = 3;
  }
  // if (middle) {
  //   middle = false;
  //   if (page == 1 ) {
  //     page=2;
  //   } else if (page ==2) {
  //     page = 1;
  //     //write value to EEPROM
  //     if (DEBUG) {
  //       Serial.print("Writing value ");
  //       Serial.print(notes[menuitem]);
  //       Serial.print(" to ");
  //       Serial.println(menuitem);
  //     }
  //     EEPROM.update(menuitem, notes[menuitem]);
  //   }
  // }

  // MIDI play
  for (uint8_t i = 0; i < inputs; i++) {
    buttonState[i] = digitalRead(inputPin[i]);
    if ((buttonState[i] == LOW) && (playing[i] == false) && (millis() - lasttrig[i] > debounce)) {
      // turn LED on:
      turnonLED(i);
      if (!DEBUG) {
        MIDI.sendNoteOn(notes[i], 100, midiChannels[i]);
      } else {
         Serial.print("ON: ");
         Serial.println(notes[i]);
       } 
      playing[i] = true;
      lasttrig[i] = millis();
    } else if ((buttonState[i] == HIGH) && (playing[i] == true)) {
      // turn LED off:
      turnoffLED(i);
       if (!DEBUG) {
        MIDI.sendNoteOff(notes[i], 100, midiChannels[i]);
       } else {
         Serial.print("OFF: ");
         Serial.println(notes[i]);
       }  
      playing[i] = false;
    }
  }
}

// Interupt routine for rotary enconder
void isr() {
  static unsigned long lastInterupTime = 0;
  unsigned long interuptTime = millis();

  // Debounce signals to 5ms
  if (interuptTime - lastInterupTime > 5) {
    if (digitalRead(encDt) == LOW) {
      // virtualPosition++;
      up = true;
    } else {
      // virtualPosition--;
      down = true;
    }
    // //Restrict rotary encoder values
    // if (page == 1) {
    //   virtualPosition = min(inputs-1, max(0, virtualPosition));
    // }
    lastInterupTime = interuptTime;
  }
}

// void checkIfSelectButtonIsPressed()
// {
//    if (selectButtonState != lastSelectButtonState) 
//   {
//     if (selectButtonState == 0) {
//       middle=true;
//     }
//     delay(50);
//   }
//    lastSelectButtonState = selectButtonState;
// }

void turnBacklightOn() {
  digitalWrite(backlightPin, LOW);
}

void turnBacklightOff() {
  digitalWrite(backlightPin, HIGH);
}

void turnonLED(uint8_t button) {
  ledBits = ledBits | myfnNumToBits(button) ;
  if (DEBUG) {
    Serial.println(ledBits, BIN);
  }
  digitalWrite(ledLatchPin, LOW);  // prepare shift register for data
  shiftOut(ledDataPin, ledClockPin, LSBFIRST, ledBits); // send data  LSBFIRST so uses outputs QB to QH (pins 2-7)
  digitalWrite(ledLatchPin, HIGH); // update display
}

void turnoffLED(uint8_t button) {
  ledBits = ledBits ^ myfnNumToBits(button) ;
  if (DEBUG) {
    Serial.println(ledBits, BIN);
  }
  digitalWrite(ledLatchPin, LOW);  // prepare shift register for data
  shiftOut(ledDataPin, ledClockPin, LSBFIRST, ledBits); // send data
  digitalWrite(ledLatchPin, HIGH); // update display
}

void updateDisplay() {
    display.setTextSize(1);
    display.clearDisplay();

    if (page == 1) {
      display.setCursor(3,3);
      display.setTextColor(BLACK);
      display.print("Select Switch");

      if (menuitem !=0) {
        display.drawRect(0, 13, 29, 18, BLACK);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.fillRect(0, 13, 29, 18, BLACK);
        display.setTextColor(WHITE, BLACK);
      }
      display.setCursor(11 , 18);
      display.print("1");

      if (menuitem !=1) {
        display.drawRect(28, 13, 29, 18, BLACK);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.fillRect(28, 13, 29, 18, BLACK);
        display.setTextColor(WHITE,BLACK);
      }
      display.setCursor(39 , 18);
      display.print("2");

      if (menuitem !=2) {
        display.drawRect(56, 13, 28, 18, BLACK);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.fillRect(56, 13, 28, 18, BLACK);
        display.setTextColor(WHITE, BLACK);
      }
      display.setCursor(67 , 18);
      display.print("3");

      if (menuitem !=3) {
        display.drawRect(0, 30, 29, 18, BLACK);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.fillRect(0, 30, 29, 18, BLACK);
        display.setTextColor(WHITE, BLACK);
      }
      display.setCursor(11 , 35);
      display.print("4");

      if (menuitem !=4) {
        display.drawRect(28, 30, 29, 18, BLACK);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.fillRect(28, 30, 29, 18, BLACK);
        display.setTextColor(WHITE, BLACK);
      }
      display.setCursor(39 , 35);
      display.print("5");

      if (menuitem !=5) {
        display.drawRect(56, 30, 28, 18, BLACK);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.fillRect(56, 30, 28, 18, BLACK);
        display.setTextColor(WHITE, BLACK);
      }
      display.setCursor(67 , 35);
      display.print("6");

  } else if (page == 2) {
    display.setCursor(19,3);
    display.setTextColor(BLACK);
    display.print("Switch:");
    display.print(menuitem+1);

    display.drawRect(0, 13, 84, 35, BLACK);
    uint8_t nameLength = strlen(noteName[notes[menuitem]-noteOffset]);
    display.setCursor((84-(nameLength*noteOffset))/2,23);  // (84 - (size*12)) /2  , ((48-14) -16) /2) + 14
    display.setTextSize(2);
    display.print(noteName[notes[menuitem]-noteOffset]);
    
  } else if (page == 3) {
    display.setCursor(3,3);
    display.setTextColor(BLACK);
    // display.print("MIDI channel");
    display.print("MIDI ch Sw: ");
    display.print(menuitem+1);
    display.drawRect(0, 13, 84, 35, BLACK);
    display.setCursor(30,23);
    display.setTextSize(2);
    if (midiChannels[menuitem] < 10) {
      display.print("0");
    }
    display.print(midiChannels[menuitem]);
  }
  display.display();
}

byte myfnNumToBits(int someNumber) {
  switch (someNumber) {
    case 0:
      return B00000001;
      break;
    case 1:
      return B00000010;
      break;
    case 2:
      return B00000100;
      break;
    case 3:
      return B00001000;
      break;
    case 4:
      return B00010000;
      break;
    case 5:
      return B00100000;
      break;
    default:
      return B00000000; // Error condition, displays three vertical bars
      break;   
  }
}