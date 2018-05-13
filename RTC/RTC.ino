#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>

LiquidCrystal lcd(8,9,4,5,6,7);

RTC_DS1307 RTC;

int x = 0;
int currentSecond = 0;
int wasSecond = 0;

void setup () {
    lcd.begin(16,2);
    Wire.begin();
    RTC.begin();
    // Check to see if the RTC is keeping time.  If it is, load the time from your computer.
    if (! RTC.isrunning()) {
      lcd.print("RTC is NOT running!");
      // This will reflect the time that your sketch was compiled
      RTC.adjust(DateTime(__DATE__, __TIME__));
    }
}
void loop () {
    DateTime now = RTC.now(); 

    currentSecond = now.second();
    
    if (currentSecond != wasSecond) {
      x = x + 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(x);
      
    }

    wasSecond = currentSecond;

    
}
