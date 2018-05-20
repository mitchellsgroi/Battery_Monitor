

/*
## TODO
    - Change the 15 second interval thing to variable
    - EEPROM Stuff, save the settings, read them and set defaults if no good
    - Temperature probes, can I have two?
    - DONE-- Fix Enter Setup --DONE
    - Make the setup screens actually work
    - Make the alarms actually work
    - Get current readings
    - DONE-- Stopwatch the AH reading for accuracy --DONE
    - truncate the AH rather than round
    - fall back to millis if rtc is fucked
    - DONE__ add rtc status screen in setup --DONE
    - DONE-- Fix display bugs in setup screens -- DONE


*/
// LIBRARIES
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

// DEFINITIONS
#define PROTO
//#define DEBUG

#define BUTTON_NONE     0
#define BUTTON_ENTER    1
#define BUTTON_SELECT   2

#define NORMAL 5
#define SETUP 6

#define VHAT 0
#define VHAO 1
#define VHAP 2
#define VLAT 3
#define VLAO 4
#define VLAP 5

#define THAS 6
#define THAT 7
#define THAO 8
#define THAP 9

#define TLAS 10
#define TLAT 11
#define TLAO 12
#define TLAP 13



// INITIALISE

#ifdef PROTO
    LiquidCrystal lcd(8,9,4,5,6,7);
    const int backlightPin = 3;
    const int buttonPin = A0;
    const int voltPin = A3;

    // VOLTAGE
    float topVolts = 5;

    // BUTTON LEVELS
    int selectVolts = 740;
    int enterVolts = 140;
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
String tempHighString = ("Maximum Temp");
String tempLowString = ("Minimum Temp");
String voltHighString = ("Maximum Volts");
String voltLowString = ("Minimum Volts");
String plus = "+";
String neg = "-";

float tempLowAlarm = 0;
byte tempLowSign = 0;
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

String tempAlarmHeader = "Temp Alarm";



#ifdef PROTO

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

    // set up the alarm values
    convertAlarms();

    // read persistant values from the EEPROM
    settingsRead();
    
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
            selectAddress = &selectCount;
            enterAddress = &backlightLevel;
            selectModulo = 100;
            enterModulo = 3;
            mainScreen();
        break;
        case 1:
            enterAddress = &tempAlarm;
            enterModulo = 4;
            tempScreen();
        break;
        case 2:
            enterAddress = &voltAlarm;
            enterModulo = 4;
            voltScreen();
        break;
        case 3:
            enterAddress = &ampHourReset;
            enterModulo = 2;
            ampHourScreen();
        break;
        case 4:
            enterAddress = &enterSetup;
            enterModulo = 2;
            enterSetupScreen();
        break;
        case 5:
            if (enterSetup) {
                selectCount++;
            }
            else {
                selectCount = 0;
            }
        break;
        case 6:
            selectAddress = &setupCount;
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

void setBack() {
    // Sets the level of the LCD backlight
    switch (backlightLevel) {
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

void screenHeader(String title) {
    lcd.setCursor(0, 0);
    lcd.print(title);
}

void mainScreen() {
    
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
    screenHeader(tempAlarmHeader);
    lcd.setCursor(0, 1);
    switch (tempAlarm) {
        case 0:
            lcd.print(tempLowAlarm, 1);
            lcd.print((char)223);
            lcd.print(" and ");
            lcd.print(tempHighAlarm, 1);
            lcd.print((char)223);
        break;
        case 1:
            lcd.print(tempHighAlarm, 1);
            lcd.print((char)223);
        break;
        case 2:
            lcd.print(tempLowAlarm, 1);
            lcd.print((char)223);
        break;
        case 3:
            lcd.print(F("Off"));  
        break;          
    }
}

void voltScreen() {
    screenHeader(F("Voltage Alarm"));
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
    screenHeader(F("Enter Setup?"));
    lcd.setCursor(0, 1);
    if (!enterSetup) {
        lcd.print(F("No "));
    }
    else {
        lcd.print(F("Yes"));
    }
}

void setupScreen() {
    switch (setupCount) {
        case 0:
            enterAddress = &tempLowSign;
            enterModulo = 2;
            screenHeader(tempLowString);
            lcd.setCursor(0, 1);
            if (tempLowSign == 1) {
                flashString(plus);
            }    
            else {
                flashString(neg);
            }
            lcd.print(tempLowTens);
            lcd.print(tempLowOnes);
            lcd.print(".");
            lcd.print(tempLowPoint);
            lcd.print((char)223);
            
        break;
        case 1:
            enterAddress = &tempLowTens;
            enterModulo = 10;
            screenHeader(tempLowString);            
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
            enterAddress = &tempLowOnes;
            enterModulo = 10;
            screenHeader(tempLowString);                    
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
            enterAddress = &tempLowPoint;
            enterModulo = 10;
            screenHeader(tempLowString);
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
            enterAddress = &tempHighSign;
            enterModulo = 2;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (tempHighSign == 1) {
                flashString(plus);
            }    
            else {
                flashString(neg);
            }
            lcd.print(tempHighTens);
            lcd.print(tempHighOnes);
            lcd.print(".");
            lcd.print(tempHighPoint);
            lcd.print((char)223);
        break;
        case 5:
            enterAddress = &tempHighTens;
            enterModulo = 10;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (tempHighSign == 1) {
                lcd.print(plus);
            }    
            else {
                lcd.print(neg);
            }
            flash(tempHighTens);
            lcd.print(tempHighOnes);
            lcd.print(".");
            lcd.print(tempHighPoint);
            lcd.print((char)223);
        break;
        case 6:
            enterAddress = &tempHighOnes;
            enterModulo = 10;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (tempHighSign == 1) {
                lcd.print(plus);
            }    
            else {
                lcd.print(neg);
            }
            lcd.print(tempHighTens);
            flash(tempHighOnes);
            lcd.print(".");
            lcd.print(tempHighPoint);
            lcd.print((char)223);
        break;
        case 7:
            enterAddress = &tempHighPoint;
            enterModulo = 10;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (tempHighSign == 1) {
                lcd.print(plus);
            }    
            else {
                lcd.print(neg);
            }
            lcd.print(tempHighTens);
            lcd.print(tempHighOnes);
            lcd.print(".");
            flash(tempHighPoint);
            lcd.print((char)223);
        break;
        case 8:
            enterAddress = &voltLowTens;
            enterModulo = 3;
            screenHeader(voltLowString);
            lcd.setCursor(0, 1);
            flash(voltLowTens);
            lcd.print(voltLowOnes);
            lcd.print(".");
            lcd.print(voltLowPoint);
            lcd.print(F("V"));
        break;
        case 9:
            enterAddress = &voltLowOnes;
            enterModulo = 10;
            screenHeader(voltLowString);
            lcd.setCursor(0, 1);
            lcd.print(voltLowTens);
            flash(voltLowOnes);
            lcd.print(".");
            lcd.print(voltLowPoint);
            lcd.print(F("V"));
        break;
        case 10:
            enterAddress = &voltLowPoint;
            enterModulo = 10;
            screenHeader(voltLowString);
            lcd.setCursor(0, 1);
            lcd.print(voltLowTens);
            lcd.print(voltLowOnes);
            lcd.print(".");
            flash(voltLowPoint);
            lcd.print(F("V"));
        break;
        case 11:
            enterAddress = &voltHighTens;
            enterModulo = 3;
            screenHeader(voltHighString);
            lcd.setCursor(0, 1);
            flash(voltHighTens);
            lcd.print(voltHighOnes);
            lcd.print(".");
            lcd.print(voltHighPoint);
            lcd.print(F("V"));
        break;
        case 12:
            enterAddress = &voltHighOnes;
            enterModulo = 10;
            screenHeader(voltHighString);
            lcd.setCursor(0, 1);
            lcd.print(voltHighTens);
            flash(voltHighOnes);
            lcd.print(".");
            lcd.print(voltHighPoint);
            lcd.print(F("V"));
        break;
        case 13:
            enterAddress = &voltHighPoint;
            enterModulo = 10;
            screenHeader(voltHighString);
            lcd.setCursor(0, 1);
            lcd.print(voltHighTens);
            lcd.print(voltHighOnes);
            lcd.print(".");
            flash(voltHighPoint);
            lcd.print(F("V"));
        break;        
        case 14:
            screenHeader(F("RTC Status"));
            if (RTC.isrunning()){
                lcd.setCursor(0, 1);
                lcd.print(F("OK    "));                    
            }
            else {
                lcd.setCursor(0, 1);
                lcd.print(F("Error: isFucked"));
            }
        break;    
        case 15:
            exitSetupScreen();
        break;
        case 16:
            convertAlarms();
            if (exitSetup) {
                lcd.clear();
                // save all settings to eeprom
                settingsWrite();
                selectCount = 0;


            }

            else {
                lcd.clear();
                setupCount = 0;
            }
    }
}
    

void exitSetupScreen() {
    enterAddress = &exitSetup;
    enterModulo = 2;
    screenHeader(F("Save and Exit?"));
    lcd.setCursor(0, 1);
    switch (exitSetup) {
        case 0:
            lcd.print(F("No "));
        break;
        case 1:
            lcd.print(F("Yes"));
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

void flashString(String s) {
    if (flashState) {
        lcd.print(s); 
    } 
    else {
        for (int i = 0; i < s.length(); i++){
          lcd.print(" ");
        }
        
    }    
}

void convertAlarms () {
    voltLowAlarm = voltLowOnes + (voltLowTens * 10) + (voltLowPoint * 0.1);
    voltHighAlarm = voltHighOnes + (voltHighTens * 10) + (voltHighPoint * 0.1);

    tempLowAlarm = tempLowOnes + (tempLowTens * 10) + (tempLowPoint * 0.1);
    tempHighAlarm = tempHighOnes + (tempHighTens * 10) + (tempHighPoint * 0.1);

    if (!tempLowSign){
      tempLowAlarm = tempLowAlarm * -1;
    }

    if (!tempHighSign) {
      tempHighAlarm = tempHighAlarm * -1;
    }
}

void settingsWrite(){
  // write all settings to the EEPROM for persistance
  EEPROM.put(TLAS, tempLowSign);
  EEPROM.put(TLAT, tempLowTens);
  EEPROM.put(TLAO, tempLowOnes);
  EEPROM.put(TLAP, tempLowPoint);

  EEPROM.put(THAS, tempHighSign);
  EEPROM.put(THAT, tempHighTens);
  EEPROM.put(THAO, tempHighOnes);
  EEPROM.put(THAP, tempHighPoint);
}

void settingsRead(){
  // read settings from EEPROM
  EEPROM.get(TLAS, tempLowSign);
  EEPROM.get(TLAT, tempLowTens);
  EEPROM.get(TLAO, tempLowOnes);
  EEPROM.get(TLAP, tempLowPoint);

  EEPROM.get(THAS, tempHighSign);
  EEPROM.get(THAT, tempHighTens);
  EEPROM.get(THAO, tempHighOnes);
  EEPROM.get(THAP, tempHighPoint);
  // do only once in setup
  // make sure to handle errors
  convertAlarms();
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
            lcd.clear();        
        break;
        case BUTTON_ENTER:
            *enterAddress = (*enterAddress + 1) % enterModulo;
            lcd.clear();
            if (enterAddress == &backlightLevel){
                setBack();
            }
        break;
    }
    // varAtAddress = (varAtAddress + 1) % globalModulo

    // varAtAddress and globalModulo are set when entering each screen
    // make function take an input of which button was pressed.
}


