// LIBRARIES
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>

// DEFINITIONS
#define PROTO
//#define DEBUG

#define BUTTON_NONE     0
#define BUTTON_ENTER    1
#define BUTTON_SELECT   2

// INITIALISE

#ifdef PROTO
    LiquidCrystal lcd(8,9,4,5,6,7);
    const int backlightPin = 3;
    const int buttonPin = A0;
#endif

RTC_DS1307 RTC;

// VARIABLES

// RTC and Timing
int currentSecond = 0;  // reading the seconds from the RTC
int wasSecond = 0;      // to compare seconds to the last loop
byte secondPassed = false;

unsigned long currentMillis = 0;  // millis for each loop for delays
unsigned long refreshMillis = 0;
const int refreshDelay = 1000;
byte refresh = false;

// voltage
float voltage = 12.5;     // final voltage to be displayed

// current
float current = 10;       // final current to be displayed

// amp hours
float ampHours = 0;     // total amp hours

// temperature
float temp = 2.5;       // final temperature reading to be displayed

// buttons
unsigned long buttonMillis = 0;
const int buttonDelay = 50;
const int buttonHyst = 40;
int rawButton = 0;
byte whichButton = 0;
byte buttonAction = false;
byte buttonPressed = 0;
byte buttonWas = 0;




// settings
int selectCount = 0;
int backlightLevel = 0;


#ifdef PROTO
    int selectVolts = 740;
    int enterVolts = 140;
#endif


void setup() {
    // NEW BEGININGS
    lcd.begin(16,2);
    Wire.begin();
    RTC.begin();

    if (!RTC.isrunning()){
        lcd.print(F("CLOCK ERROR")); 
    }

    // PRINT TO LCD AS FAUX SPLASH SCREEN
    lcd.print(F("  The Battery   "));
    lcd.setCursor(0, 1);
    lcd.print(F("    Monitor     "));

    // PIN THE TAIL ON THE DONKY
    pinMode(backlightPin, OUTPUT);

    pinMode(buttonPin, INPUT);

    digitalWrite(backlightPin, HIGH);

    lcd.clear();
    
}
void loop() {
    // capture millis at the start of the loop
    currentMillis = millis();

    if ((currentMillis - refreshMillis) > refreshDelay) {
        refresh = true;
        refreshMillis = currentMillis;
    }

    else {
        refresh = false;
    }

    checkSecond();

    // COUNT AMP HOURS
    // just counting seconds at the moment
    if (secondPassed) {
        ampHours++;
    }
    

    // run the debug function if in debug mode
    #ifdef DEBUG
        debug();
    #endif

    // Buttons
    if ((currentMillis - buttonMillis) > buttonDelay) {

        whichButton = checkButton();

        buttonMillis = currentMillis;

        if (buttonAction == true) {
            switch (whichButton) {
                case BUTTON_NONE:
                    break;
                case BUTTON_SELECT:
                    lcd.clear();
                    selectCount = (selectCount + 1) % 4;
                    break;
                case BUTTON_ENTER:
                    switch (selectCount) {
                        case 0:
                            backlightLevel = (backlightLevel + 1) % 3;
                            setBack(backlightLevel);
                    }
                    break;    
            }
            buttonAction = false;
        }
    }

    switch (selectCount) {
        case 0:
            mainScreen();
        break;
        case 1:
            tempScreen();
        break;
        case 2:
            voltScreen();
        break;
        case 3:
            ampHourScreen();
        break;
    }
    

        

    

    
    
}



// DEBUG CODE
void debug() {
    // Print the button voltages to set
    rawButton = analogRead(buttonPin);
    lcd.setCursor(0, 0);
    lcd.print(rawButton);

    lcd.setCursor(0, 1);
    lcd.print("s");
    lcd.print(selectCount);

}

byte checkButton(){

    rawButton = analogRead(buttonPin);
    
    if (rawButton > (selectVolts - buttonHyst) && rawButton < (selectVolts + buttonHyst)) {
        buttonPressed = BUTTON_SELECT;
    }
    else if (rawButton > (enterVolts - buttonHyst) && rawButton < (enterVolts + buttonHyst)) {
        buttonPressed = BUTTON_ENTER;
    }
    else {
        buttonPressed = BUTTON_NONE;
    }

    if (buttonWas == BUTTON_NONE && buttonPressed != BUTTON_NONE) {
        buttonAction = true;
    }

    buttonWas = buttonPressed;

    return buttonPressed;
}

void setBack(int level) {
    // Sets the level of the LCD backlight
    switch (level) {
        case 0:
            digitalWrite(backlightPin, HIGH);
        break;
        case 1:
            analogWrite(backlightPin, 127);
        break;
        case 2:
            digitalWrite(backlightPin, LOW);
        break;
    }
}

void checkSecond() {
    DateTime now = RTC.now();
    currentSecond = now.second();

    if (currentSecond != wasSecond) {
        secondPassed = true;
    }
    else {
        secondPassed = false;
    }

    wasSecond = currentSecond;
}


void mainScreen() {

    // VOLTAGE
    lcd.setCursor(0, 0);
    lcd.print("V:");
    if (refresh) {
        lcd.print(voltage, 1);
    }

    // CURRENT
    lcd.setCursor(0, 1);
    lcd.print("A:");
    if (refresh) {
        lcd.print(current, 1);
    }

    // TEMPERATURE
    lcd.setCursor(8, 0);
    lcd.print("T:");
    if (refresh) {
        lcd.print(temp, 1);
    }

    // AMP HOURS
    lcd.setCursor(8, 1);
    lcd.print("%:");
    lcd.print(ampHours, 0);
}

void tempScreen() {
    lcd.setCursor(0, 0);
    lcd.print("TEMP SCREEN");
}

void voltScreen() {
    lcd.setCursor(0, 0);
    lcd.print("VOLT SCREEN");
}

void ampHourScreen() {
    lcd.setCursor(0, 0);
    lcd.print("AH SCREEN");
}

