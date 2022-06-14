// YWROBOT
// Compatible with the Arduino IDE 1.0
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Library version:1.5.1

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() { 
   lcd.begin(16, 2);
   lcd.setBacklightPin(3, POSITIVE);
   lcd.setBacklight(HIGH);
   lcd.home();
   lcd.print("Hello, world!");
   lcd.setCursor(9, 1);
   lcd.print("OK,OK");
}

void loop() {
  
}
