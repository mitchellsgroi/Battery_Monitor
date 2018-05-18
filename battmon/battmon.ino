/*
## TODO
    - Change the 15 second interval thing to variable
    - EEPROM Stuff, save the settings, read them and set defaults if no good
    - Temperature probes, can I have two?
    - Fix Enter Setup --DONE
    - Make the setup screens actually work
    - Make the alarms actually work
    - Get current readings
    - Stopwatch the AH reading for accuracy
    - truncate the AH rather than round


*/
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

#define NORMAL 5
#define SETUP 6


// INITIALISE

#ifdef PROTO
    LiquidCrystal lcd(8,9,4,5,6,7);
    const int backlightPin = 3;
    const int buttonPin = A0;
    const int voltPin = A3;

    // VOLTAGE
    float topVolts = 5;
#endif

RTC_DS1307 RTC;

// VARIABLES

// RTC and Timing
int currentSecond = 0;  // reading the seconds from the RTC
int wasSecond = 0;      // to compare seconds to the last loop
byte secondPassed = false;
byte fifteenSecond = false;

unsigned long currentMillis = 0;  // millis for each loop for delays
unsigned long refreshMillis = 0;
const int refreshDelay = 1000;
byte refresh = false;

const int numReadings = 50;

// voltage

float kVolts = 0.97;
float voltage = 12.5;     // final voltage to be displayed
int rawVolts = 0;
int arrayVolts[numReadings];
float totalVolts = 0;
int iVolts = 0;

byte voltAlarm = 0;
float voltHigh = 15.1;
float voltLow  = 11.8;




// current
float current = 60;       // final current to be displayed

// amp hours
float ampHours = 0;     // total amp hours
byte ampHourReset = 0;

// temperature
float temp = 2.5;       // final temperature reading to be displayed
byte tempAlarm = 0; 
float tempLow = -2.5;
float tempHigh = 3.5;

// buttons
unsigned long buttonMillis = 0;
const int buttonDelay = 50;
const int buttonHyst = 40;
int rawButton = 0;
byte whichButton = 0;
byte buttonAction = false;
byte buttonPressed = 0;
byte buttonWas = 0;

int screenReturn = 0;
int screenMax = 3000;




// settings
byte selectCount = 0;
byte backlightLevel = 0;
byte setupCount = 0;
byte tempSetupCount = 0;
byte numScreens = 5;

byte enterSetup = false;
byte exitSetup = 0;

// AH SYMBOL
byte ahSymbol[8] = {
  B01000,
  B10100,
  B11100,
  B10100,
  B00000,
  B00101,
  B00111,
  B00101,
};


#ifdef PROTO
    int selectVolts = 740;
    int enterVolts = 140;
#endif


void setup() {
    // NEW BEGININGS
    lcd.createChar(2, ahSymbol);
    lcd.begin(16,2);
    Wire.begin();
    RTC.begin();

    if (!RTC.isrunning()){
        lcd.print(F("CLOCK ERROR"));
        delay(5000); 
    }

    // PRINT TO LCD AS FAUX SPLASH SCREEN
    lcd.print(F("  The Battery   "));
    lcd.setCursor(0, 1);
    lcd.print(F("    Monitor     "));
    delay(300);

    // PIN THE TAIL ON THE DONKY
    pinMode(backlightPin, OUTPUT);

    pinMode(buttonPin, INPUT);
    pinMode(voltPin, INPUT);

    digitalWrite(backlightPin, HIGH);
    digitalWrite(voltPin, LOW);

    // CLEAR AVERAGING ARRAYS
    for (int i = 0; i < numReadings; i++) {
        arrayVolts[i] = 0;
    }

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

    // VOLTAGE
    rawVolts = analogRead(voltPin);

    totalVolts -= arrayVolts[iVolts];
    arrayVolts[iVolts] = rawVolts;
    totalVolts += arrayVolts[iVolts];
    iVolts++;

    if (iVolts >= numReadings) {
        iVolts = 0;
    }

    voltage = ((totalVolts / numReadings) * (topVolts / 1023.0)) * kVolts;

    
    // COUNT AMP HOURS
    if (secondPassed) {
        ampHours += current / 3600;

        if (ampHours < 0) {
            ampHours = 0;
        }
    }

    // TEMPERATURE
    if (fifteenSecond) {
        // ping the sensor(s) for the temp reading
        // just counting for now
        temp++;
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
            screenReturn = 0;
            switch (whichButton) {
                case BUTTON_NONE:
                    break;
                case BUTTON_SELECT:
                    lcd.clear();
                    if (selectCount < 5) {
                        selectCount = (selectCount + 1) % numScreens;
                        
                    }
                    else {
                        setupCount = (setupCount + 1) % 5;
                    }
                    break;
                case BUTTON_ENTER:
                    switch (selectCount) {
                        case 0:
                            backlightLevel = (backlightLevel + 1) % 3;
                            setBack(backlightLevel);
                        break;
                        case 1:
                            lcd.clear();
                            tempAlarm = (tempAlarm + 1) % 4;  
                        break;
                        case 2:
                            lcd.clear();
                            voltAlarm = (voltAlarm + 1) % 4;
                        break;
                        case 3:
                            ampHourReset = !ampHourReset;
                        break;
                        case 4:
                            enterSetup = !enterSetup;
                        break;
                        case 5:
                            switch (setupCount) {
                                case 0:
                                    tempSetupCount = (tempSetupCount + 1) % 2;
                                break;
                                case 5:
                                    exitSetup = !exitSetup;
                                break;
                            }
                            
                            
                        break;
                    }  
            }
            buttonAction = false;
        }
    }

    if (selectCount != 0) {
        screenReturn++;
        if (screenReturn >= screenMax) {
            lcd.clear();
            selectCount = 0;
            screenReturn = 0;
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
        case 4:
            enterSetupScreen();
        break;
        case 5:
            switch (setupCount) {
                case 0:
                    tempSetupScreen();
                break;
                case 1:
                    voltSetupScreen();
                break;
                case 2:
                    exitSetupScreen();
                break;
                case 3:
                    if (exitSetup) {
                        selectCount = 0;
                    }
                else {
                    setupCount = 0;
                }
                break;
                   
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
    if (!RTC.isrunning()){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("CLOCK ERROR"));
        delay(1000);
    }
    DateTime now = RTC.now();
    currentSecond = now.second();

    if (currentSecond != wasSecond) {
        secondPassed = true;
    }
    else {
        secondPassed = false;
    }

    // change the magic 15 here to a variable that can be changed
    if (((currentSecond % 15) == 0) && currentSecond != wasSecond) {
        fifteenSecond = true;
    }
    else {
        fifteenSecond = false;
    }
    
    wasSecond = currentSecond;        
}


void mainScreen() {
    if (ampHourReset) {
        ampHours = 0;
        ampHourReset = false;
    }

    enterSetup = false;
    exitSetup = false;
    
    // VOLTAGE
    lcd.setCursor(0, 0);
    lcd.print(F("V:"));
    if (refresh) {
        lcd.print(voltage, 1);
    }

    // CURRENT
    lcd.setCursor(0, 1);
    lcd.print(F("A:"));
    if (refresh) {
        lcd.print(current, 1);
    }

    // TEMPERATURE
    lcd.setCursor(8, 0);
    lcd.print(F("T:"));
    if (refresh) {
        lcd.print(temp, 1);
    }

    // AMP HOURS
    lcd.setCursor(8, 1);
    lcd.write(byte(2));
    lcd.print(F(":"));
    lcd.print(ampHours, 1);
}

void tempScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Temp Alarm"));
    lcd.setCursor(0, 1);
    switch (tempAlarm) {
        case 0:
            lcd.print(tempLow, 1);
            lcd.print((char)223);
            lcd.print(" and ");
            lcd.print(tempHigh, 1);
            lcd.print((char)223);
        break;
        case 1:
            lcd.print(tempHigh, 1);
            lcd.print((char)223);
        break;
        case 2:
            lcd.print(tempLow, 1);
            lcd.print((char)223);
        break;
        case 3:
            lcd.print(F("Off"));  
        break;          
    }
}

void voltScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Voltage Alarm"));
    lcd.setCursor(0, 1);
    switch (voltAlarm) {
        case 0:
            lcd.print(voltLow, 1);
            lcd.print(F("V"));
            lcd.print(" and ");
            lcd.print(voltHigh, 1);
            lcd.print(F("V"));
        break;
        case 1:
            lcd.print(voltHigh, 1);
            lcd.print(F("V"));
        break;
        case 2:
            lcd.print(voltLow, 1);
            lcd.print(F("V"));
        break;
        case 3:
            lcd.print(F("Off"));  
        break;          
    }
}

void ampHourScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Reset Amp Hours?"));
    lcd.setCursor(0, 1);
    if (!ampHourReset) {
        lcd.print(F("No "));
    }
    else {
        lcd.print(F("Yes"));
    }
}

void enterSetupScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Enter Setup?"));
    lcd.setCursor(0, 1);
    if (!enterSetup) {
        lcd.print("No ");
        numScreens = NORMAL;
    }
    else {
        lcd.print("Yes");
        numScreens = SETUP;
    }
}

void tempSetupScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Temp Alarm"));
    switch (tempSetupCount) {
        case 0:
            lcd.setCursor(10, 0);
            lcd.print(" High");
            lcd.setCursor(0, 1);
            lcd.print("+10.2C");
        break;
        case 1:
            lcd.setCursor(10, 0);
            lcd.print(" Low ");
            lcd.setCursor(0, 1);
            lcd.print("-12.3C");
        break;    
    }
}

void voltSetupScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Volt Alarm Limits"));
    lcd.setCursor(0, 1);
    lcd.print("Low:");
    lcd.print("00.0V High: 00.0V");
    
}

void exitSetupScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Exit Setup?"));
    lcd.setCursor(0, 1);
    switch (exitSetup) {
        case 0:
            lcd.print("No ");
        break;
        case 1:
            lcd.print("Yes");
        break;
    }
}

void floatDetect() {
    // if the batteries voltage has been above 14, current has stayed very low, and the voltage is still above 13.2ish
        // then battery is floating
        // reset amp hours to zero and keep them there while floating

    // once current goes high or voltage goes lower
        // then battery is not floating
        // reset the maxvolts
}

void chargeDetect() {
    // if charge input is high or current is in negative
        // charging is true
        // turn on the LED       
}


