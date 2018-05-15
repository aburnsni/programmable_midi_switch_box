// MIDI tx
// lcd pins 3, 4, 5, 6, 7, 8
// rotary enc pins 2, A6, A7
// switch inputs A0, A1, A2, A3, A4, A5
// hc595 driven leds 9, 10, 11
#include <MIDI.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

//Variables for inputs
const int inputs = 6;
const int inputPin[inputs] = {A0, A1, A2, A3, A4, A5};
int buttonState[inputs] = {1, 1, 1, 1, 1, 1};
bool playing[inputs] = {false, false, false, false, false, false};  //Is note currently playing
unsigned long lasttrig[inputs];
unsigned long debounce = 10;

// Variables for rotary encoder
const int encClk = 2; // Needs Interupt pin
const int encDt = A6;
const int encSw = A7;
bool up = false;
bool down = false;
int lastCount = 0;
volatile int virtualPosition = 0;
bool swState = true;

// Pins for hc595 to drive LEDs
const int ledClockPin = 9;
const int ledLatchPin = 10;
const int ledDataPin = 11;

// Note names matched to MIDI value
int midiChannel = 1;
const int numNotes = 84;
String noteName[numNotes] = {
  "", "", "", "", "", "", "", "", "", "", "", "",
  "C0", "C#0", "D0", "Eb0", "E0", "F0", "F#0", "G0", "G#0", "A0", "Bb0", "B0",
  "C1", "C#1", "D1", "Eb1", "E1", "F1", "F#1", "G1", "G#1", "A1", "Bb1", "B1",
  "C2", "C#2", "D2", "Eb2", "E2", "F2", "F#2", "G2", "G#2", "A2", "Bb2", "B2",
  "C3", "C#3", "D3", "Eb3", "E3", "F3", "F#3", "G3", "G#3", "A3", "Bb3", "B3",
  "C4", "C#4", "D4", "Eb4", "E4", "F4", "F#4", "G4", "G#4", "A4", "Bb4", "B4",
  "C5", "C#5", "D5", "Eb5", "E5", "F5", "F#5", "G5", "G#5", "A5", "Bb5", "B5"
};

// temp value for EEPROM reading
int value;

// default MIDI notes - replaced from EEPROM
int notes[inputs] = {60, 62, 64, 65, 67, 69};

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

// Initialise display
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4,3 ); //Download the latest Adafruit Library in order to use this constructor
bool backlight = true;
int contrast = 60;
const int backlightPin = 8;

// Menu
int page = 1;


void setup() {
  MIDI.begin();
  Serial.begin(9600);
  // Switch inputs with pullups
  for (int i = 0; i < inputs; i++) {
    pinMode (inputPin[i], INPUT_PULLUP);
  }

  // Set last trigger time for each input
  for (int i = 0; i < inputs; i++) {
    lasttrig[i] = millis();
  }

  //read EEPROM values
  for (int i = 0; i < inputs; i++) {
    value = EEPROM.read(i);
    if (value > 11 && value < 128) {
      notes[i] = value;
    }
  }

  // rotary encoder I/Os
  pinMode(encClk, INPUT);
  pinMode(encDt, INPUT);
  pinMode(encSw, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encClk), isr, LOW);

  // hc595 I/Os
  pinMode(ledClockPin, OUTPUT);
  pinMode(ledLatchPin, OUTPUT);
  pinMode(ledDataPin, OUTPUT);

  // display
  pinMode(backlightPin, OUTPUT);
  turnBacklightOn();
  
  display.begin();
  display.clearDisplay();
  display.setContrast(contrast);
  display.display();


//debug info to Serial
  for (int i = 0; i< inputs; i++) {
    Serial.print(notes[i]);
    Serial.print("\t");
  }
  Serial.println();

  updateDisplay();
}

void loop () {
  if (virtualPosition != lastCount) {
    Serial.print(virtualPosition > lastCount ? "Up :" : "Down :");
    Serial.println(virtualPosition);
    // byte bits = myfnNumToBits(virtualPosition) ;
    // myfnUpdateDisplay(bits);    // display alphanumeric digit
    lastCount = virtualPosition;
    updateDisplay();
  }

  swState = digitalRead(encSw);
  if (swState == LOW) {
    // modeSelect();
  }

  for (uint8_t i = 0; i < inputs; i++) {
    buttonState[i] = digitalRead(inputPin[i]);
    if ((buttonState[i] == HIGH) && (playing[i] == false) && (millis() - lasttrig[i] > debounce)) {
      // turn LED on:
      turnonLED(i);
      MIDI.sendNoteOn(notes[i], 100, midiChannel);
      playing[i] = true;
      lasttrig[i] = millis();
    } else if ((buttonState[i] == LOW) && (playing[i] == true)) {
      // turn LED off:
      turnoffLED(i);
      MIDI.sendNoteOff(notes[i], 100, midiChannel);
      playing[i] = false;
    }
  }


  //write value to EEPROM
  // EEPROM.update(address, value)
}

// Interupt routine for rotary enconder
void isr() {
  static unsigned long lastInterupTime = 0;
  unsigned long interuptTime = millis();

  // Debounce signals to 5ms
  if (interuptTime - lastInterupTime > 5) {
    if (digitalRead(encDt) == LOW) {
      virtualPosition--;
    } else {
      virtualPosition++;
    }
    //Restrict rotary encoder values
    if (page == 1) {
      virtualPosition = min(inputs-1, max(0, virtualPosition));
    }
    lastInterupTime = interuptTime;
  }
}

void turnBacklightOn() {
  digitalWrite(backlightPin, LOW);
}

void turnBacklightOff() {
  digitalWrite(backlightPin, HIGH);
}

void turnonLED(int button) {
  
}

void turnoffLED(int button) {

}

void updateDisplay() {
    display.setTextSize(1);
    display.clearDisplay();

    display.setCursor(1,3);
    display.setTextColor(BLACK);
    display.print("Select Switch");

    if (virtualPosition !=0) {
      display.drawRect(0, 13, 29, 18, BLACK);
      display.setTextColor(BLACK, WHITE);
    } else {
      display.fillRect(0, 13, 29, 18, BLACK);
      display.setTextColor(WHITE, BLACK);
    }
    display.setCursor(11 , 18);
    display.print("1");

    if (virtualPosition !=1) {
      display.drawRect(28, 13, 29, 18, BLACK);
      display.setTextColor(BLACK, WHITE);
    } else {
      display.fillRect(28, 13, 29, 18, BLACK);
      display.setTextColor(WHITE,BLACK);
    }
    display.setCursor(39 , 18);
    display.print("2");

    if (virtualPosition !=2) {
      display.drawRect(56, 13, 28, 18, BLACK);
      display.setTextColor(BLACK, WHITE);
    } else {
      display.fillRect(56, 13, 28, 18, BLACK);
      display.setTextColor(WHITE, BLACK);
    }
    display.setCursor(67 , 18);
    display.print("3");

    if (virtualPosition !=3) {
      display.drawRect(0, 30, 29, 18, BLACK);
      display.setTextColor(BLACK, WHITE);
    } else {
      display.fillRect(0, 30, 29, 18, BLACK);
      display.setTextColor(WHITE, BLACK);
    }
    display.setCursor(11 , 35);
    display.print("4");

    if (virtualPosition !=4) {
      display.drawRect(28, 30, 29, 18, BLACK);
      display.setTextColor(BLACK, WHITE);
    } else {
      display.fillRect(28, 30, 29, 18, BLACK);
      display.setTextColor(WHITE, BLACK);
    }
    display.setCursor(39 , 35);
    display.print("5");

    if (virtualPosition !=5) {
      display.drawRect(56, 30, 28, 18, BLACK);
      display.setTextColor(BLACK, WHITE);
    } else {
      display.fillRect(56, 30, 28, 18, BLACK);
      display.setTextColor(WHITE, BLACK);
    }
    display.setCursor(67 , 35);
    display.print("6");

   display.display();
}