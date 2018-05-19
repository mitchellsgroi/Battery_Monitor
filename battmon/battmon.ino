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
    - fall back to millis if rtc is fucked
    - add rtc status screen in setup
    - Fix display bugs in setup screens


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

// ALARMS
float tempLowAlarm = 0;
byte tempLowSign = -1;
byte tempLowTens = 0;
byte tempLowOnes = 3;
byte tempLowPoint = 5;

float tempHighAlarm = 0;
byte tempHighSign = 1;
byte tempHighTens = 0;
byte tempHighOnes = 4;
byte tempHighPoint = 5;

float voltLowAlarm = 0;
byte voltLowTens = 1;
byte voltLowOnes = 1;
byte voltLowPoint = 9;

float voltHighAlarm = 0;
byte voltHighTens = 1;
byte voltHighOnes = 5;
byte voltHighPoint = 2;


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

byte *selectAddress;
byte *enterAddress;

byte selectModulo = 0;
byte enterModulo = 0;

// CURSOR AND ALARM FLASHING
byte flashState = true;
unsigned long lastFlashMillis = 0;
const int flashDelay = 250;


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
        lcd.print(F("RTC Resetting"));
        delay(1000);
        RTC.adjust(DateTime(__DATE__, __TIME__)); 
    }

    if (!RTC.isrunning()){
        lcd.print(F("RTC ERROR"));
    }

    // PRINT TO LCD AS FAUX SPLASH SCREEN
    lcd.print(F("  The Battery   "));
    lcd.setCursor(0, 1);
    lcd.print(F("    Monitor     "));
    delay(10);

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
    doFlashing();

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
            buttonIncrement();
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
            setupScreen();
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
//    if (!RTC.isrunning()){
//        lcd.clear();
//        lcd.setCursor(0, 0);
//        lcd.print(F("CLOCK ERROR"));
//        delay(1000);
//    }
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

void settingButtons() {
    switch (setupCount) {
        case 0:
            tempSetupCount = (tempSetupCount + 1) % 8;
        break;
        case 12:
            lcd.clear();
            exitSetup = !exitSetup;
        break;
    }
}


void mainScreen() {

    selectAddress = &selectCount;
    enterAddress = &backlightLevel;

    selectModulo = numScreens;
    enterModulo = 4;
    
    if (ampHourReset) {
        ampHours = 0;
        ampHourReset = false;
    }

    enterSetup = false;
    exitSetup = false;
    setupCount = 0;
    
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
            lcd.print(voltLowAlarm, 1);
            lcd.print(F("V"));
            lcd.print(" and ");
            lcd.print(voltHighAlarm, 1);
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

void setupScreen() {
    switch (setupCount) {
        case 0:
            lcd.setCursor(0, 0);
            lcd.print("Temp Alarm Low");
            lcd.setCursor(0, 1);
            if (tempLowSign == 1) {
                flash("+");
            }    
            else {
                flash("-");
            }
            lcd.print(tempLowTens);
            lcd.print(tempLowOnes);
            lcd.print(".");
            lcd.print(tempLowPoint);
            lcd.print((char)223);
            
        break;
        case 1:
            lcd.setCursor(0, 1);
            if (tempLowSign == 1) {
                lcd.print("+");
            }    
            else {
                lcd.print("-");
            }
            flash(tempLowTens);
            lcd.print(tempLowOnes);
            lcd.print(".");
            lcd.print(tempLowPoint);
            lcd.print((char)223);
        break;
        case 2:
            lcd.setCursor(0, 1);
            if (tempLowSign == 1) {
                lcd.print("+");
            }    
            else {
                lcd.print("-");
            }
            lcd.print(tempLowTens);
            flash(tempLowOnes);
            lcd.print(".");
            lcd.print(tempLowPoint);
            lcd.print((char)223);
        break;        
        case 3:
            lcd.setCursor(0, 1);
            if (tempLowSign == 1) {
                lcd.print("+");
            }    
            else {
                lcd.print("-");
            }
            lcd.print(tempLowTens);
            lcd.print(tempLowOnes);
            lcd.print(".");
            flash(tempLowPoint);
            lcd.print((char)223);
        break;            
        case 4:
            lcd.setCursor(11, 0);
            lcd.print("High");
            lcd.setCursor(0, 1);
            lcd.print("C12.3");
        break;
        case 5:
            lcd.setCursor(0, 1);
            lcd.print("-C2.3");
        break;
        case 6:
            lcd.setCursor(0, 1);
            lcd.print("-1C.3");
        break;
        case 7:
            lcd.setCursor(0, 1);
            lcd.print("-12.C");
        break;
        case 8:
            lcd.setCursor(0, 0);
            lcd.print(F("Volt Alarm Low"));
            lcd.setCursor(0, 1);
            lcd.print("C1.9");
        break;
        case 9:
            lcd.setCursor(0, 1);
            lcd.print("1C.9");
        break;
        case 10:
            lcd.setCursor(0, 1);
            lcd.print("11.C");
        break;
        case 11:
            lcd.setCursor(0, 0);
            lcd.print(F("RTC Status      "));
            if (RTC.isrunning()){
                lcd.setCursor(0, 1);
                lcd.print(F("OK    "));                    
            }
            else {
                lcd.setCursor(0, 1);
                lcd.print(F("Error: isFucked"));
            }
        break;    
        case 12:
            exitSetupScreen();
        break;
        case 13:
            convertAlarms();
            if (exitSetup) {
                lcd.clear();
                selectCount = 0;


            }

            else {
                lcd.clear();
                setupCount = 0;
            }
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



void doFlashing() {
    if ((currentMillis - lastFlashMillis) > flashDelay) {
        flashState = !flashState;
        lastFlashMillis = currentMillis;
    }
}

void flash(byte x) {
    if (flashState) {
        lcd.print(x); 
    }
    else {
        lcd.print(" ");
    }
}

void convertAlarms () {
    voltLowAlarm = voltLowOnes + (voltLowTens * 10) + (voltLowPoint * 0.1);
    voltHighAlarm = voltHighOnes + (voltHighTens * 10) + (voltHighPoint * 0.1);

    tempLowAlarm = (tempLowOnes + (tempLowTens * 10) + (tempLowPoint * 0.1)) * tempLowSign;
    tempHighAlarm = (tempHighOnes + (tempHighTens * 10) + (tempHighPoint * 0.1)) * tempHighSign;
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

void buttonIncrement() {
    switch (whichButton) {
        case BUTTON_NONE:
        break;
        case BUTTON_SELECT:
            *selectAddress = (*selectAddress + 1) % selectModulo;        
        break;
        case BUTTON_ENTER:
            *enterAddress = (*enterAddress + 1) % enterModulo;
        break;
    }
    // varAtAddress = (varAtAddress + 1) % globalModulo

    // varAtAddress and globalModulo are set when entering each screen
    // make function take an input of which button was pressed.
}


