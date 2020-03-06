void turnBacklightOn() {
  digitalWrite(backlightPin, HIGH);
}

void turnBacklightOff() {
  digitalWrite(backlightPin, LOW);
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