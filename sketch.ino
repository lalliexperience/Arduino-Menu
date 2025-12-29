#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PIN DEFINITIONS ---
const int BTN_UP    = 2;
const int BTN_DOWN  = 3;
const int BTN_ENTER = 4;
const int BTN_BACK  = 5;

// --- MENU TRACKING ---
enum MenuState { MAIN_MENU, SETTINGS_SUB, SYSTEM_SUB };
MenuState currentState = MAIN_MENU;
int cursor = 0; 

// --- DATA STORAGE ---
String deviceName = "NANO_01"; 

// Charset: Standard alphabet, numbers, space, and the return arrow (\x11)
const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 \x11";

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  drawMenu(); 
}

void loop() {
  if (digitalRead(BTN_UP) == LOW) {
    cursor--;
    if (cursor < 0) cursor = 0; 
    drawMenu();
    delay(200);
  }

  if (digitalRead(BTN_DOWN) == LOW) {
    cursor++; 
    drawMenu();
    delay(200);
  }

  if (digitalRead(BTN_BACK) == LOW) {
    if (currentState != MAIN_MENU) {
      currentState = MAIN_MENU;
      cursor = 0;
      drawMenu();
    }
    delay(200);
  }

  if (digitalRead(BTN_ENTER) == LOW) {
    handleSelection();
    delay(200);
  }
}

// --- RENDERING ENGINE ---
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  switch (currentState) {
    case MAIN_MENU:
      if (cursor > 1) cursor = 1;
      display.println(F("--- MAIN MENU ---"));
      drawItem("1. Settings", 0, 18);
      drawItem("2. System Info", 1, 30);
      break;

    case SETTINGS_SUB:
      if (cursor > 1) cursor = 1;
      display.println(F("--- SETTINGS ---"));
      drawItem("Edit System Name", 0, 18);
      drawItem("Back to Main", 1, 30);
      break;

    case SYSTEM_SUB:
      if (cursor > 0) cursor = 0;
      display.println(F("--- SYSTEM INFO ---"));
      display.setCursor(0, 15);
      display.print("Current Name:");
      display.setCursor(0, 25);
      // Wrapped in quotes to see spaces
      display.print("\""); display.print(deviceName); display.println("\""); 
      display.setCursor(0, 40);
      display.println("Firmware: v1.0.7");
      drawItem("Back", 0, 52); 
      break;
  }
  display.display();
}

void drawItem(String text, int pos, int yBase) {
  display.setCursor(0, yBase);
  if (cursor == pos) display.print("> ");
  else display.print("  ");
  display.println(text);
}

// --- LOGIC ENGINE ---
void handleSelection() {
  switch (currentState) {
    case MAIN_MENU:
      if (cursor == 0) currentState = SETTINGS_SUB;
      else if (cursor == 1) currentState = SYSTEM_SUB;
      cursor = 0;
      break;

    case SETTINGS_SUB:
      if (cursor == 0) {
        while(digitalRead(BTN_ENTER) == LOW); 
        delay(100);
        deviceName = startTypingRoutine("System Name", deviceName);
      }
      else if (cursor == 1) currentState = MAIN_MENU;
      cursor = 0;
      break;

    case SYSTEM_SUB:
      currentState = MAIN_MENU;
      cursor = 0;
      break;
  }
  drawMenu();
}

// --- TYPING ROUTINE ---
String startTypingRoutine(String title, String oldValue) {
  String newString = "";
  int charIndex = 0;
  bool typingActive = true;

  while (typingActive) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Edit: "); display.println(title);
    // Wrapped old value in quotes
    display.print("Old: \""); display.print(oldValue); display.println("\"");
    display.drawLine(0, 18, 128, 18, SSD1306_WHITE);

    display.setCursor(0, 25);
    display.print("Input: \""); 
    display.print(newString);
    
    // Blinking cursor
    if ((millis() / 300) % 2 == 0) {
       if (charset[charIndex] == ' ') display.print("_"); 
       else display.print(charset[charIndex]);
    } else {
       display.print(" "); 
    }
    display.print("\""); // Closing quote for input line
    
    display.setCursor(0, 50);
    display.print("Select \x11 to Save"); 
    display.display();

    if (digitalRead(BTN_UP) == LOW) {
      charIndex--;
      if (charIndex < 0) charIndex = strlen(charset) - 1;
      delay(150);
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      charIndex++;
      if (charIndex >= strlen(charset)) charIndex = 0;
      delay(150);
    }
    if (digitalRead(BTN_ENTER) == LOW) {
      if (charset[charIndex] == '\x11') {
        typingActive = false;
      } else {
        newString += charset[charIndex];
      }
      while(digitalRead(BTN_ENTER) == LOW); 
      delay(50);
    }
    if (digitalRead(BTN_BACK) == LOW) {
      if (newString.length() > 0) {
        newString.remove(newString.length() - 1);
        while(digitalRead(BTN_BACK) == LOW);
      } else return oldValue;
      delay(200);
    }
  }

  // Success screen with quotes
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Saved!");
  display.print("Val: \""); display.print(newString); display.println("\"");
  display.display();
  delay(1500);
  
  return (newString == "") ? oldValue : newString;
}