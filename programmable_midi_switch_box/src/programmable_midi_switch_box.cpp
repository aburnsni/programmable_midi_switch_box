// MIDI tx
// lcd pins 5, 6, 7, 8, 9
// rotary enc pins 2, 3, 4
// switch inputs A0, A1, A2, A3, A4, A5
// hc595 driven leds 10, 11, 12
#include <Arduino.h>

#include <MIDI.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "DualFunctionButton.h"
#include <avr/pgmspace.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include "notes.h"
#include "ablogo.h"
#include "variables.h"
#include "numtobits.h"
#include "leds.h"

Encoder encKnob(encClk, encDt);

DualFunctionButton button(encSw, 1000, INPUT_PULLUP);

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

Adafruit_PCD8544 display = Adafruit_PCD8544(8, 7, 6, 5); // pin CS (labelled CE) tied to GND, BL on Pin 9

#include "display.h"

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
  if (DEBUG || usbmidi) {
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
    if (value >= 0 && value <= 127) {
      notes[i] = value;
    }
  }
  for (uint8_t i = 0; i < inputs; i++) {
    value = EEPROM.read(midiAddress + i);
    if (value >= 1 && value <=16) { //valid midiChannel
      midiChannels[i] = value;
    }
  }
  for (uint8_t i = 0; i < inputs; i++) {
    value = EEPROM.read(volumeAddress + i);
    if (value >= 0 && value <=127) { //valid volume
      volumes[i] = value;
    }
  }

  // rotary encoder I/Os
  pinMode(encSw, INPUT_PULLUP);

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

  long newEncPosition = encKnob.read()/2;
  if (newEncPosition != encPostition) {
    if (newEncPosition > encPostition) {
      change = -1;
    } else if (newEncPosition < encPostition) {
      change = 1;
    }
    encPostition = newEncPosition;
  }

  if (change != 0) {
    if (DEBUG) {
      Serial.print(page);
      Serial.print("\t");
      Serial.print(menuitem);
      Serial.print("\t");
      Serial.print(notes[menuitem]);
      Serial.print("\t");
      Serial.print(midiChannels[menuitem]);
      Serial.print("\t");
      Serial.print(volumes[menuitem]);
      Serial.print("\t");
      Serial.println(change);
    }
    // Rotary encoder up
    if (page == 1) {
      if ((menuitem >= 0) && (menuitem <= inputs-1)) {
        if (!((menuitem == 0 && change < 0) || (menuitem == inputs-1 && change > 0))) {
          menuitem = menuitem + change;
        }
      }
    } else if (page == 2) {
      if ((notes[menuitem] >= 0) && (notes[menuitem] <= 127)) {
        if (!((notes[menuitem] == 0 && change < 0) || (notes[menuitem] == 127 && change > 0))) {
          notes[menuitem] = notes[menuitem] + change;
        }
      }
    } else if (page == 3) {
      if ((midiChannels[menuitem] >= 1) && (midiChannels[menuitem] <= 16)) {
        if (!((midiChannels[menuitem] == 1 && change < 0) || (midiChannels[menuitem] == 16 && change > 0))) {
          midiChannels[menuitem] = midiChannels[menuitem] + change;
        }
      }
    } else if (page == 4) {
      if ((volumes[menuitem] >= 1) && (volumes[menuitem] <= 127)) {
        if (!((volumes[menuitem] == 1 && change < 0) || (volumes[menuitem] == 127 && change > 0))) {
          volumes[menuitem] = volumes[menuitem] + change;
        }
      }
    }
    updateDisplay();
    change = 0;
  }
  //Rotary endcoder press
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
      page = 2; //Return to Switch setting;
      //write value to EEPROM
      if (DEBUG) {
        Serial.print("Writing MIDI channel value ");
        Serial.print(midiChannels[menuitem]);
        Serial.print(" to ");
        Serial.println(menuitem);
      }
      EEPROM.update(midiAddress + menuitem, midiChannels[menuitem]);
    } else if (page ==4) {
      page = 2; //Return to Switch setting;
      //write value to EEPROM
      if (DEBUG) {
        Serial.print("Writing volume value ");
        Serial.print(volumes[menuitem]);
        Serial.print(" to ");
        Serial.println(menuitem);
      }
      EEPROM.update(volumeAddress + menuitem, volumes[menuitem]);
    } else if (page == 9) {
      page = 1;
    }
    updateDisplay();
  }
  if (button.longPress()) {
    if (DEBUG) {
      Serial.println("Long Press");
    }
    if (page == 1) {
      page = 9;
    } else if (page == 2) {
      page = 3;
    } else if (page == 3) {
      page = 4;
    }
    updateDisplay();
  }

  // MIDI play
  for (uint8_t i = 0; i < inputs; i++) {
    buttonState[i] = digitalRead(inputPin[i]);
    if ((buttonState[i] == LOW) && (playing[i] == false) && (millis() - lasttrig[i] > debounce)) {
      // turn LED on:
      turnonLED(i);
      if (!DEBUG) {
        MIDI.sendNoteOn(notes[i], volumes[i], midiChannels[i]);
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

  if (millis()-backlightTrig > backlightTimeOut) {
    turnBacklightOff();
  }
}