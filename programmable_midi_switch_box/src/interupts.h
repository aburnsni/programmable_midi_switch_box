// Interupt routine for rotary enconder
void isr() {
  static unsigned long lastInterupTime = 0;
  unsigned long interuptTime = millis();

  // Debounce signals to 5ms
  if (interuptTime - lastInterupTime > 5) {
    if (digitalRead(encDt) == LOW) {
      up = true;
    } else {
      down = true;
    }
    lastInterupTime = interuptTime;
  }
}