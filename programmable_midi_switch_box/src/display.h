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
    uint8_t nameLength = strlen(strcpy_P(buffer, (char*)pgm_read_word(&(note_table[notes[menuitem]]))));
    display.setCursor((84-(nameLength*12))/2,23);  // (84 - (size*12)) /2  , ((48-14) -16) /2) + 14
    display.setTextSize(2);
    display.print(strcpy_P(buffer, (char*)pgm_read_word(&(note_table[notes[menuitem]]))));
    display.setTextSize(1);
    display.setCursor(3,30);
    display.print("Ch");
    display.setCursor(3,38);
    if (midiChannels[menuitem] < 10) {
      display.print("0");
    }
    display.print(midiChannels[menuitem]);
    display.setCursor(64,30);
    display.print("Vol");
    display.setCursor(64,38);

    if (volumes[menuitem] < 100) {
      display.print(" ");
    }
    if (volumes[menuitem] < 10) {
      display.print(" ");
    }
    display.print(volumes[menuitem]);
    
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

  } else if (page == 4) {
    display.setCursor(6,3);
    display.setTextColor(BLACK);
    display.print("Volume Sw: ");
    display.print(menuitem+1);
    display.drawRect(0, 13, 84, 35, BLACK);
    display.setCursor(24,23);
    display.setTextSize(2);
    if (volumes[menuitem] < 100) {
      display.print("0");
    }
    if (volumes[menuitem] < 10) {
      display.print("0");
    }
    display.print(volumes[menuitem]);

  } else if (page == 9) {
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    for (uint8_t i = 0; i < inputs; i++) {
      display.setCursor(0,(i*8));
      display.print(i);
      display.setCursor(12,(i*8));

      display.print(strcpy_P(buffer, (char*)pgm_read_word(&(note_table[notes[i]]))));
      display.setCursor(42,(i*8));
      if (midiChannels[i] < 10) {
        display.print("0");
      }
      display.print(midiChannels[i]);
      display.setCursor(60,(i*8));
      if (volumes[i] < 100) {
        display.print(" ");
      }
      if (volumes[i] < 10) {
        display.print(" ");
      }
      display.print(volumes[i]);
    }
  }
  display.display();

  backlightTrig = millis();  // Reset backlight timer each time the encoder triggers
  turnBacklightOn();
}