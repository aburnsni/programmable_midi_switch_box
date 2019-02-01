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

uint8_t lastCount = 0;
bool swState = true;

// Pins for hc595 to drive LEDs
const uint8_t ledClockPin = 10;
const uint8_t ledLatchPin = 11;
const uint8_t ledDataPin = 12;
byte ledBits = B00000000;

// Note names matched to MIDI value
uint8_t midiAddress = inputs; //use address space after inputs
uint8_t volumeAddress = inputs*2; //use address space after inputs and channels

// buffer to read strings from PROGMEM
char buffer[5]; 

// temp value for EEPROM reading
uint8_t value;

// default MIDI notes - replaced from EEPROM
uint8_t notes[inputs] = {60, 61, 13, 65, 67, 69};
uint8_t midiChannels[inputs] = {1, 1, 1, 1, 1, 1};
uint8_t volumes[inputs] = {100, 100, 100, 100, 100, 100};

// display
bool backlight = true;
uint8_t contrast = 60;
const uint8_t backlightPin = 9;
unsigned long backlightTrig = millis();
const int backlightTimeOut = 30 * 1000;  // 30 second timeout on backlight

// Menu
uint8_t page = 1;
uint8_t menuitem = 0;