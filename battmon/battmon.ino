// LIBRARIES
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>

// DEFINITIONS
#define PROTO
#define DEBUG

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

unsigned long currentMillis = 0;  // millis for each loop for delays

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


int selectCount = 0;
int enterCount = 0;




#ifdef PROTO
    int selectVolts = 140;
    int enterVolts = 740;
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
                    selectCount = (selectCount + 1) % 20;
                    break;
                case BUTTON_ENTER:
                    lcd.clear();
                    enterCount = (enterCount + 1) % 20;
                    break;    
            }
            buttonAction = false;
        }
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
    lcd.setCursor(5, 1);
    lcd.print("e");
    lcd.print(enterCount);
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

