

/*
## TODO
    - DONE-- Change the 15 second interval thing to variable --DONE
    - DONE-- EEPROM Stuff, save the settings, read them and set defaults if no good --DONE
    - Temperature probes, can I have two?
    - DONE-- Fix Enter Setup --DONE
    - DONE-- Make the setup screens actually work --DONE
    - DONE-- Make the alarms actually work --DONE
    - DONE-- Get current readings --DONE
    - DONE-- Stopwatch the AH reading for accuracy --DONE
    - DONE-- truncate the AH rather than round --DONE
    - DONE-- fall back to millis if rtc is fucked --DONE
    - DONE-- add rtc status screen in setup --DONE
    - DONE-- Fix display bugs in setup screens -- DONE
    - DONE--Put all the settings into an array of bytes --DONE
    - DONE-- Handle wrong settings --DONE
    - Comment everything
    - write float detection
    - DONE-- Write charge detection current & charge signal --DONE
    - Done-- Write battery capacity tester --Done
    - Make work on old PCB
    - Make work on new PCB
    - DONE-- Address amp hour reset bug in capacity tester --DONE
    - DONE-- Change float so only triggered after set time (15min) --DONE

## BUGS
    - Erratic current readings in test vehicle when running fridge. Switches between positive and negative triggering the charge LED
    - Float detection will reset amp hours as voltage settles when charger is turned off mid charge.

*/
// LIBRARIES
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DEFINITIONS
//#define PROTO
#define PCBONE
//#define PCBTWO

//#define DEBUG
//#define TESTING

#define BUTTON_NONE     0
#define BUTTON_ENTER    1
#define BUTTON_SELECT   2

#define NORMAL 5
#define SETUP 6

#define SETTING_SIZE 20

#define VHAT 0  // Volt High Alarm Tens
#define VHAO 1  // Volt High Alarm Ones 
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

#define VA 14
#define TA 15

#define WA 16   // Which Alarm 



// INITIALISE

#ifdef PROTO
    #define ONE_WIRE_BUS 11

    
    
    LiquidCrystal lcd(8,9,4,5,6,7);
    const int backlightPin = 3;
    const int buttonPin = A0;
    const int voltPin = A3;
    const int ledPin = 12;
    const int buzzerPin = 13;
    const int relayPin = 10;

    const int currentVrefPin = A1;
    const int currentVoutPin = A2;


    // VOLTAGE
    float topVolts = 5;

    // BUTTON LEVELS
    int selectVolts = 740;
    int enterVolts = 140;
    
    // FLOAT DETECTION
    float maxVoltage = 0;
    float floatVoltage = 3.0;
    float chargeVoltage = 4.2;
    bool floating = false;

#endif

#ifdef PCBONE // Original PCB
    
    #define ONE_WIRE_BUS 8
    
    LiquidCrystal lcd(12,11,5,4,3,2);

    const int chargePin = 0;
    const int ledPin = 1;
    const int buzzerPin = 9;
    const int backlightPin = 10;
    const int relayPin = 13;
     
    const int buttonPin = A0;
    const int currentVoutPin = A1;
    const int currentVrefPin = A2;
    const int voltPin = A3;

    // VOLTAGE
    float topVolts = 28.5;

    // CURRENT
    float kCurrent = 0.0;

    // BUTTON VOLTAGES
    int selectVolts = 790;
    int enterVolts = 260;

    // FLOAT DETECTION
    float maxVoltage = 0;
    float floatVoltage = 13.4;
    float chargeVoltage = 14.1;
    bool floating = false;
    int floatSeconds = 0;   
    int floatSecondsMax = 600;
     
#endif

#ifdef PCBTWO
    #define ONE_WIRE_BUS 8
    
    LiquidCrystal lcd(12,11,5,4,3,2);

    const int ledPin = 1;
    const int buzzerPin = 9;
    const int backlightPin = 10;
    const int relayPin = 7;
     
    const int buttonPin = A0;
    const int currentVoutPin = A1;
    const int currentVrefPin = A2;
    const int voltPin = A3;

    // VOLTAGE
    float topVolts = 28.5;

    // BUTTON VOLTAGES
    int selectVolts = 800;
    int enterVolts = 260;

    // FLOAT DETECTION
    float maxVoltage = 0;
    float floatVoltage = 13.4;
    float chargeVoltage = 14.1;
    bool floating = false;
    
     
#endif
    

RTC_DS1307 RTC;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// VARIABLES

// RTC and Timing
int currentSecond = 0;  // reading the seconds from the RTC
int wasSecond = 0;      // to compare seconds to the last loop
byte secondPassed = false;
byte fifteenSecond = false;
byte fifteenSecondCount = 0;
byte fifteenSecondMax = 15;

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

String no = "No";
String yes = "Yes";

byte settings[SETTING_SIZE];

byte tempAlarm = 0;
float tempLowAlarm = 0;
float tempHighAlarm = 0;
byte tempAlarming = false;

byte voltAlarm = 0;
float voltLowAlarm = 0;
float voltHighAlarm = 0;
byte voltAlarming = false;

int alarmCount = 0;
int alarmMax = 2000;

bool ledOn = false;

// current
int vRefRaw = 0;
int totalVref = 0;
int arrayVref[numReadings];
int iVref = 0;
float vRefAverage = 0;

int vOutRaw = 0;
int totalVout = 0;
int arrayVout[numReadings];
int iVout = 0;
float vOutAverage = 0;
//float vRef = 0;
//float vOut = 0;
float thisCurrent = 0;
float totalCurrent = 0;
float arrayCurrent[numReadings];
int iCurrent = 0;

float current = 0;       // final current to be displayed

// voltage

float kVolts = 0.97;
float voltage = 12.5;     // final voltage to be displayed
int rawVolts = 0;
int arrayVolts[numReadings];
float totalVolts = 0;
int iVolts = 0;

// CHARGING

int chargeHigh = LOW;
bool isCharging = false;


// Battery Testing
byte batteryTest = 0;
byte startTesting = 0;
byte isTesting = 0;
byte doneTesting = true;
float testResult = 0;
bool relayOn = false;


// amp hours
int ampHours = 0;     // total amp hours
float ampHourFloat = 0;
byte ampHourReset = 0;

// temperature
float temp = 2.5;       // final temperature reading to be displayed 
float tempLow = -2.5;
float tempHigh = 3.5;

// buttons
unsigned long buttonMillis = 0;
const int buttonDelay = 50;
const int buttonHyst = 10; // How many volts above or below the ones set above
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


void setup() {
    // NEW BEGININGS
     Wire.begin();
    lcd.createChar(2, ahSymbol);
    lcd.begin(16,2);
    
   
    RTC.begin();
    sensors.begin();

    if (!RTC.isrunning()){
        lcd.clear();
        lcd.print(F("RTC Resetting"));
        delay(200);
        RTC.adjust(DateTime(__DATE__, __TIME__)); 
    }

    if (!RTC.isrunning()){
        lcd.clear();
        lcd.print(F("RTC ERROR"));
        delay(2000);
    }

    // PRINT TO LCD AS FAUX SPLASH SCREEN
    lcd.clear();
    lcd.print(F("  The Battery   "));
    lcd.setCursor(0, 1);
    lcd.print(F("    Monitor     "));
    delay(300);

    // PIN THE TAIL ON THE DONKY
    pinMode(backlightPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(relayPin, OUTPUT);   
    pinMode(buttonPin, INPUT);
    #ifdef PCBONE
        pinMode(buttonPin, INPUT_PULLUP);
    #endif
    pinMode(voltPin, INPUT);
    pinMode(currentVrefPin, INPUT);
    pinMode(currentVoutPin, INPUT);
    

    digitalWrite(backlightPin, HIGH);
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    digitalWrite(relayPin, LOW);


    // CLEAR AVERAGING ARRAYS
    for (int i = 0; i < numReadings; i++) {
        arrayVolts[i] = 0;
        arrayCurrent[i] = 0;
        arrayVref[i] = 0;
        arrayVout[i] = 0;
    }

    // Read the temps sensor on startup
    sensors.requestTemperatures();
    // ping the sensor(s) for the temp reading
    temp = sensors.getTempCByIndex(0);

    // set up the alarm values
    convertAlarms();

    // read persistant values from the EEPROM
    settingsRead();
    
    lcd.clear();
    
}
void loop() {
    // Do all the millis related functions
    checkMillis();
    // Do all the second related functions
    checkSecond();
    // Check the voltage of the battery
    batteryVoltage();
  
    // Check the current in or out of the battery
    batteryCurrent();
    // Check if the battery is floating
    floatDetect();
    // Check if the battery is charging
    charging();
    // Check the temperature
    temperature();     
    // Count the Amp Hours
    countAmpHours();
    // Trigger any alarms
    triggerAlarm();
    // Set off buzzer or LED
    soundAlarm();
    // Check if a button has been presseds
    buttons();
    // Print info the the LCD
    displayScreen();
    // Return to home screen if no activity
    returnHome();   
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
void checkMillis() {
    // capture millis at the start of the loop
    currentMillis = millis();
    
    if ((currentMillis - refreshMillis) > refreshDelay) {
        refresh = true;
        refreshMillis = currentMillis;
    }

    else {
        refresh = false;
    }
    if ((currentMillis - lastFlashMillis) > flashDelay) {
        flashState = !flashState;
        lastFlashMillis = currentMillis;
    }
}

void checkSecond() {
    if (RTC.isrunning()){

        DateTime now = RTC.now();
        currentSecond = now.second();
    
        if (currentSecond != wasSecond) {
            secondPassed = true;
        }
        else {
            secondPassed = false;
        }
    
        // change the magic 15 here to a variable that can be changed
        if (((currentSecond % fifteenSecondMax) == 0) && currentSecond != wasSecond) {
            fifteenSecond = true;
        }
        else {
            fifteenSecond = false;
        }
        
        wasSecond = currentSecond;
    }
    else {
         if (refresh) {
            secondPassed = true;
            if (fifteenSecondCount >= fifteenSecondMax) {
                fifteenSecond = true;
                fifteenSecondCount = 0;
            }
            fifteenSecondCount++;       
         }
         else {
            secondPassed = false;
            fifteenSecond = false;
         }
    }
}

void batteryVoltage() {
    // Take the analog input
    rawVolts = analogRead(voltPin);
    // Take the old value from the total
    totalVolts -= arrayVolts[iVolts];
    // Put the new value in the array
    arrayVolts[iVolts] = rawVolts;
    // Add the new value to the array
    totalVolts += arrayVolts[iVolts];
    // increment the array index
    iVolts++;
    // reset the index at the end of the array
    if (iVolts >= numReadings) {
        iVolts = 0;
    }
    delay[1];

    // calculate average voltage
    voltage = ((totalVolts / numReadings) * (topVolts / 1023.0)) * kVolts;    
}



void batteryCurrent() {
    
    vRefRaw = analogRead(currentVrefPin);
    vOutRaw = analogRead(currentVoutPin);

    // take the old value from the total
    totalVref -= arrayVref[iVref];
    // Insert the new value
    arrayVref[iVref] = vRefRaw;
    // Add this new value to the total
    totalVref += arrayVref[iVref];
    // Go to next index
    iVref++;
    // Reset index if at the end of array
    if (iVref >= numReadings) {
        iVref = 0;
    }
    delay[1];

    vRefAverage = totalVref / numReadings;


        // take the old value from the total
    totalVout -= arrayVout[iVout];
    // Insert the new value
    arrayVout[iVout] = vOutRaw;
    // Add this new value to the total
    totalVout += arrayVout[iVout];
    // Go to next index
    iVout++;
    // Reset index if at the end of array
    if (iVout >= numReadings) {
        iVout = 0;
    }
    delay[1];

    vOutAverage = totalVout / numReadings;
    

    thisCurrent = 160 * (vOutAverage - vRefAverage);

    // take the old value from the total
    totalCurrent -= arrayCurrent[iCurrent];
    // Insert the new value
    arrayCurrent[iCurrent] = thisCurrent;
    // Add this new value to the total
    totalCurrent += arrayCurrent[iCurrent];
    // Go to next index
    iCurrent++;
    // Reset index if at the end of array
    if (iCurrent >= numReadings) {
        iCurrent = 0;
    }
    delay[1];

    // calculate the average
    current = ((totalCurrent / numReadings) * (5.0 / 1023.0)) + kCurrent;

    if (current < 0.4 && current > -0.4) {
        current = 0;
    }
    
    
}

void countAmpHours() {
    if (secondPassed) {
        ampHourFloat += current / 3600;

        if (ampHourFloat < 0) {
            ampHourFloat = 0;
        }

    ampHours = ampHourFloat;
    }
}

void temperature() {
    // TEMPERATURE
    if (fifteenSecond && !selectCount && !batteryTest) {
        sensors.requestTemperaturesByIndex(0);
        // ping the sensor(s) for the temp reading
        temp = sensors.getTempCByIndex(0);
        // just counting for now
//        temp++;
    }    
}

void triggerAlarm() {
    switch(settings[TA]) {
        case 0:
            tempAlarming = false;
        break;
        case 1:
            if (temp >= tempHighAlarm || temp <= tempLowAlarm) {
                tempAlarming = true;
                alarmCount = alarmMax;
            }
            else {
                tempAlarming = false;
            }
        break;
        case 2:
            if (temp >= tempHighAlarm) {
                tempAlarming = true;
                alarmCount = alarmMax;
            }
            else {
                tempAlarming = false;
            }
        break;    
        case 3:
            if (temp <= tempLowAlarm) {
                tempAlarming = true;
                alarmCount = alarmMax;
            }
            else {
                tempAlarming = false;
            }
        break;
    }

    switch(settings[VA]) {
        case 0:
            voltAlarming = false;
        break;
        case 1:
            if (voltage >= voltHighAlarm || voltage <= voltLowAlarm) {
                voltAlarming = true;
                alarmCount = alarmMax;
            }
            else {
                voltAlarming = false;
            }
        break;
        case 2:
            if (voltage >= voltHighAlarm) {
                voltAlarming = true;
                alarmCount = alarmMax;    
            }
            else {
                voltAlarming = false;
            }
        break;
        case 3:
            if (voltage <= voltLowAlarm) {
                voltAlarming = true;
                alarmCount = alarmMax;
            }
            else {
                voltAlarming = false;
            }
        break;
    }

    if ((!voltAlarming || !tempAlarming) && alarmCount > 0) {
        alarmCount-- ;
    }
}

void soundAlarm() {
    if (voltAlarming || tempAlarming || alarmCount) {
        switch (settings[WA]) {
            case 0:
             setLed(0);
             digitalWrite(buzzerPin, LOW);
            break;
            case 1:
                flashLed();
                flashBuzzer();
            break;
            case 2:
                flashLed();
                digitalWrite(buzzerPin, LOW);
            break;
            case 3:
                flashBuzzer();
                setLed(0);
            break;
        }
    }
    else {
        if (isCharging) {
            setLed(1);
        }
        else {
            setLed(0);
        }
        digitalWrite(buzzerPin, LOW);
    }
    
}

void buttons() {
    // Check the button press wasn't a bounce
    if ((currentMillis - buttonMillis) > buttonDelay) {
        // check which button it was
        whichButton = checkButton();
        // reset the debounce timer
        buttonMillis = currentMillis;
        // check if the button pressed should trigger an action
        if (buttonAction == true) {
            // reset the activity timer
            screenReturn = 0;
            // do the action that is required
            buttonIncrement();
            // reset the action trigger
            buttonAction = false;
        }
    }
}

void returnHome() {
    // If not on the home screen    
    if (selectCount != 0) {
        // Increment the count
        screenReturn++;
        // If count has reached the max value
        if (screenReturn >= screenMax) {
            // Clear the LCD
            lcd.clear();
            // Return to the home screen
            selectCount = 0;
            // Reset the count
            screenReturn = 0;
        }
    }
}

void displayScreen() {
        switch (selectCount) {
        case 0:
            if (!batteryTest) {
                enterAddress = &backlightLevel;
                enterModulo = 3;
                mainScreen();
            }
            else {
                enterAddress = &startTesting;
                enterModulo = 2;
                batteryTester();
                testerScreen();
            }
            selectAddress = &selectCount;
            selectModulo = 100;
            
        break;
        case 1:
            enterAddress = &settings[TA];
            enterModulo = 4;
            tempScreen();
        break;
        case 2:
            enterAddress = &settings[VA];
            enterModulo = 4;
            voltScreen();
        break;
        case 3:
            enterAddress = &settings[WA];
            enterModulo = 4;
            alarmScreen();
        break;
        case 4:
            enterAddress = &ampHourReset;
            enterModulo = 2;
            ampHourScreen();
        break;
        case 5:
            enterAddress = &enterSetup;
            enterModulo = 2;
            enterSetupScreen();
        break;
        case 6:
            if (doneTesting) {
                batteryTest = 0;
            }
            if (enterSetup) {
                selectCount++;
            }
            else {
                settingsWrite();
                selectCount = 0;
            }
        break;
        case 7:
            selectAddress = &setupCount;
            setupScreen();
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
    if (refresh) {
        lcd.clear();
    }
    
    // VOLTAGE
    lcd.setCursor(0, 0);
    if (voltAlarming) {
        flashString(F("V:"));
    }
    else {
        lcd.print(F("V:"));
    }
    
    if (refresh) {
        lcd.print(voltage, 1);
    }

    // CURRENT
    lcd.setCursor(0, 1);
    lcd.print(F("A:"));
    if (refresh) {
        lcd.print(current, 1);
        //lcd.print(vRef, 1);
    }

    // TEMPERATURE
    lcd.setCursor(8, 0);
    if (tempAlarming) {
        flashString(F("T:"));
    }
    else {
        lcd.print(F("T:"));
    }
    
    if (refresh) {
        lcd.print(temp, 1);
    }

    // AMP HOURS
    lcd.setCursor(8, 1);
    lcd.write(byte(2));
    lcd.print(F(":"));
    lcd.print(ampHours, 1);

    // INFO CHAR
    lcd.setCursor(15, 1);
    if (floating) {
        lcd.print(F("F"));
    }
}

void tempScreen() {
    screenHeader(tempAlarmHeader);
    lcd.setCursor(0, 1);
    switch (settings[TA]) {
        case 0:
            lcd.print(F("Off"));  
        break;         
        case 1:
            lcd.print(F("<"));
            lcd.print(tempLowAlarm, 1);
            lcd.print((char)223);
            lcd.print(" and >");
            lcd.print(tempHighAlarm, 1);
            lcd.print((char)223);
        break;
        case 2:
            lcd.print(F(">"));
            lcd.print(tempHighAlarm, 1);
            lcd.print((char)223);
        break;
        case 3:
            lcd.print(F("<"));
            lcd.print(tempLowAlarm, 1);
            lcd.print((char)223);
        break;
         
    }
}

void voltScreen() {
    screenHeader(F("Voltage Alarm"));
    lcd.setCursor(0, 1);
    switch (settings[VA]) {
        case 0:
            lcd.print(F("Off"));
        break;
        case 1:
            lcd.print(F(">"));
            lcd.print(voltHighAlarm, 1);
            lcd.print(F("V & <"));
            lcd.print(voltLowAlarm, 1);
            lcd.print(F("V"));
            
        break;
        case 2:
            lcd.print(F(">"));
            lcd.print(voltHighAlarm, 1);
        break;
        case 3:
            lcd.print(F("<"));
            lcd.print(voltLowAlarm, 1);  
        break;          
    }
}

void alarmScreen() {
    screenHeader(F("Which Alarm?"));
    lcd.setCursor(0, 1);
    switch (settings[WA]) {
        case 0:
            lcd.print(F("Off"));
        break;
        case 1:
            lcd.print(F("LED & Buzzer"));
        break;
        case 2:
            lcd.print(F("LED Only"));
        break;
        case 3:
            lcd.print(F("Buzzer Only"));
        break;
    }
}

void ampHourScreen() {
    screenHeader(F("Reset Amp Hours?"));
    lcd.setCursor(0, 1);
    if (!ampHourReset) {
        lcd.print(no);
    }
    else {
        lcd.print(yes);
    }
}

void enterSetupScreen() {
    screenHeader(F("Enter Setup?"));
    lcd.setCursor(0, 1);
    if (!enterSetup) {
        lcd.print(no);
    }
    else {
        lcd.print(yes);
    }
}

void setupScreen() {
    switch (setupCount) {
        case 0:
            enterAddress = &settings[TLAS];
            enterModulo = 2;
            screenHeader(tempLowString);
            lcd.setCursor(0, 1);
            if (settings[TLAS] == 1) {
                flashString(plus);
            }    
            else {
                flashString(neg);
            }
            lcd.print(settings[TLAT]);
            lcd.print(settings[TLAO]);
            lcd.print(".");
            lcd.print(settings[TLAP]);
            lcd.print((char)223);
            
        break;
        case 1:
            enterAddress = &settings[TLAT];
            enterModulo = 10;
            screenHeader(tempLowString);            
            lcd.setCursor(0, 1);
            if (settings[TLAS] == 1) {
                lcd.print("+");
            }    
            else {
                lcd.print("-");
            }
            flash(settings[TLAT]);
            lcd.print(settings[TLAO]);
            lcd.print(".");
            lcd.print(settings[TLAP]);
            lcd.print((char)223);
        break;
        case 2:
            enterAddress = &settings[TLAO];
            enterModulo = 10;
            screenHeader(tempLowString);                    
            lcd.setCursor(0, 1);
            if (settings[TLAS] == 1) {
                lcd.print("+");
            }    
            else {
                lcd.print("-");
            }
            lcd.print(settings[TLAT]);
            flash(settings[TLAO]);
            lcd.print(".");
            lcd.print(settings[TLAP]);
            lcd.print((char)223);
        break;        
        case 3:
            enterAddress = &settings[TLAP];
            enterModulo = 10;
            screenHeader(tempLowString);
            lcd.setCursor(0, 1);
            if (settings[TLAS] == 1) {
                lcd.print("+");
            }    
            else {
                lcd.print("-");
            }
            lcd.print(settings[TLAT]);
            lcd.print(settings[TLAO]);
            lcd.print(".");
            flash(settings[TLAP]);
            lcd.print((char)223);
        break;            
        case 4:
            enterAddress = &settings[THAS];
            enterModulo = 2;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (settings[THAS] == 1) {
                flashString(plus);
            }    
            else {
                flashString(neg);
            }
            lcd.print(settings[THAT]);
            lcd.print(settings[THAO]);
            lcd.print(".");
            lcd.print(settings[THAP]);
            lcd.print((char)223);
        break;
        case 5:
            enterAddress = &settings[THAT];
            enterModulo = 10;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (settings[THAS] == 1) {
                lcd.print(plus);
            }    
            else {
                lcd.print(neg);
            }
            flash(settings[THAT]);
            lcd.print(settings[THAO]);
            lcd.print(".");
            lcd.print(settings[THAP]);
            lcd.print((char)223);
        break;
        case 6:
            enterAddress = &settings[THAO];
            enterModulo = 10;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (settings[THAS] == 1) {
                lcd.print(plus);
            }    
            else {
                lcd.print(neg);
            }
            lcd.print(settings[THAT]);
            flash(settings[THAO]);
            lcd.print(".");
            lcd.print(settings[THAP]);
            lcd.print((char)223);
        break;
        case 7:
            enterAddress = &settings[THAP];
            enterModulo = 10;        
            screenHeader(tempHighString);
            lcd.setCursor(0, 1);
            if (settings[THAS] == 1) {
                lcd.print(plus);
            }    
            else {
                lcd.print(neg);
            }
            lcd.print(settings[THAT]);
            lcd.print(settings[THAO]);
            lcd.print(".");
            flash(settings[THAP]);
            lcd.print((char)223);
        break;
        case 8:
            enterAddress = &settings[VLAT];
            enterModulo = 3;
            screenHeader(voltLowString);
            lcd.setCursor(0, 1);
            flash(settings[VLAT]);
            lcd.print(settings[VLAO]);
            lcd.print(".");
            lcd.print(settings[VLAP]);
            lcd.print(F("V"));
        break;
        case 9:
            enterAddress = &settings[VLAO];
            enterModulo = 10;
            screenHeader(voltLowString);
            lcd.setCursor(0, 1);
            lcd.print(settings[VLAT]);
            flash(settings[VLAO]);
            lcd.print(".");
            lcd.print(settings[VLAP]);
            lcd.print(F("V"));
        break;
        case 10:
            enterAddress = &settings[VLAP];
            enterModulo = 10;
            screenHeader(voltLowString);
            lcd.setCursor(0, 1);
            lcd.print(settings[VLAT]);
            lcd.print(settings[VLAO]);
            lcd.print(".");
            flash(settings[VLAP]);
            lcd.print(F("V"));
        break;
        case 11:
            enterAddress = &settings[VHAT];
            enterModulo = 3;
            screenHeader(voltHighString);
            lcd.setCursor(0, 1);
            flash(settings[VHAT]);
            lcd.print(settings[VHAO]);
            lcd.print(".");
            lcd.print(settings[VHAP]);
            lcd.print(F("V"));
        break;
        case 12:
            enterAddress = &settings[VHAO];
            enterModulo = 10;
            screenHeader(voltHighString);
            lcd.setCursor(0, 1);
            lcd.print(settings[VHAT]);
            flash(settings[VHAO]);
            lcd.print(".");
            lcd.print(settings[VHAP]);
            lcd.print(F("V"));
        break;
        case 13:
            enterAddress = &settings[VHAP];
            enterModulo = 10;
            screenHeader(voltHighString);
            lcd.setCursor(0, 1);
            lcd.print(settings[VHAT]);
            lcd.print(settings[VHAO]);
            lcd.print(".");
            flash(settings[VHAP]);
            lcd.print(F("V"));
        break;
        case 14:
            testerSettingScreen();
        break;            
        case 15:
            screenHeader(F("RTC Status"));
            if (RTC.isrunning()){
                lcd.setCursor(0, 1);
                lcd.print(F("OK"));                    
            }
            else {
                lcd.setCursor(0, 1);
                lcd.print(F("Error: isFucked"));
            }
        break;    
        case 16:
            exitSetupScreen();
        break;
        case 17:
            convertAlarms();
            checkSettings();
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



void testerSettingScreen() {
    enterAddress = &batteryTest;
    enterModulo = 2;
    screenHeader(F("Test Battery?"));
    lcd.setCursor(0, 1);
    if (batteryTest) {
        lcd.print(yes);
        if (!isTesting) {
           ampHourReset = true; // Only reset ampHours if not testing 
        }
        doneTesting = false;
    }
    else {
        lcd.print(no);
        ampHourReset = false;
        doneTesting = true;
    }

    lcd.setCursor(7, 1);
    lcd.print(testResult);
    lcd.print("AH");
    
}

void testerScreen() {
    if (ampHourReset) {
        ampHours = 0;
        ampHourReset = false;
    }

    enterSetup = false;
    exitSetup = false;
    setupCount = 0;
    if (refresh) {
        lcd.clear();
    }
    
    // VOLTAGE
    lcd.setCursor(0, 0);
    if (voltAlarming) {
        flashString(F("V:"));
    }
    else {
        lcd.print(F("V:"));
    }
    
    if (refresh) {
        lcd.print(voltage, 1);
    }

    // CURRENT
    lcd.setCursor(8, 0);
    lcd.print(F("A:"));
    if (refresh) {
        lcd.print(current, 1);
        //lcd.print(vRef, 1);
    }

        // AMP HOURS
    lcd.setCursor(0, 1);
    lcd.write(byte(2));
    lcd.print(F(":"));
    lcd.print(ampHourFloat, 1);

    // INFORMATION
    lcd.setCursor(8, 1);
    switch(startTesting) {
        case 0:
            if(isTesting) {
                lcd.print(F("Pause  "));
            }
            else if (doneTesting) {
                lcd.print(F("Done   "));
            }
            else {
                lcd.print(F("Ready  "));
            }
        break;
        case 1:
            lcd.print(F("Testing"));
        break;
    }

    
}



// BATTERY CAPACITY TESTE LOGIC
// Should run only when doing battery test but regardless of screen displayed
void batteryTester() {
    // if startTesting is true but not already testing
    if (startTesting && !isTesting) {
        setRelay(1); // Turn on the relay
        isTesting = true; // we are now testing
        doneTesting = false; // we are not done testing
    }

    // this is "Pause" state
    // turn off the relay during a test
    else if (!startTesting && isTesting) {
        setRelay(0); //Turn off the relay
    }

    // keep the relay on so long as the test has started
    else if (startTesting && isTesting) {
        setRelay(1); // Turn on the relay;
    }

    // finish the test once the volts go low
    if (voltage <= voltLowAlarm && isTesting) {
        setRelay(0); //Turn off the relay
        testResult = ampHourFloat; // save the result
        // save test result to EEPROM 
        isTesting = false; // we are no longer testing
        startTesting = false; // we are not testing
        doneTesting = true; // the test is done
        
    }
        
}

// RELAY CONTROL
void setRelay(byte on) {
    // if sent on command and relay not already on
    if (on && !relayOn) {
        digitalWrite(relayPin, HIGH); // turn on the relay
        relayOn = true; // save on state
    }

    // if sent off command and relay is still on
    else if (!on && relayOn) {
        digitalWrite(relayPin, LOW); // turn off the relay
        relayOn = false; // save the state
    }
}

// LED CONTROL
void setLed(byte on) {
    // if sent on command and relay not already on
    if (on && !ledOn) {
        digitalWrite(ledPin, HIGH); // turn on the relay
        ledOn = true; // save on state
    }

    // if sent off command and relay is still on
    else if (!on && ledOn) {
        digitalWrite(ledPin, LOW); // turn off the relay
        ledOn = false; // save the state
    }
}

void exitSetupScreen() {
    enterAddress = &exitSetup;
    enterModulo = 2;
    screenHeader(F("Save and Exit?"));
    lcd.setCursor(0, 1);
    switch (exitSetup) {
        case 0:
            lcd.print(no);
        break;
        case 1:
            lcd.print(yes);
        break;
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
    voltLowAlarm = settings[VLAO] + (settings[VLAT] * 10) + (settings[VLAP] * 0.1);
    voltHighAlarm = settings[VHAO] + (settings[VHAT] * 10) + (settings[VHAP] * 0.1);

    tempLowAlarm = settings[TLAO] + (settings[TLAT] * 10) + (settings[TLAP] * 0.1);
    tempHighAlarm = settings[THAO] + (settings[THAT] * 10) + (settings[THAP] * 0.1);

    if (!settings[TLAS]){
      tempLowAlarm = tempLowAlarm * -1;
    }

    if (!settings[THAS]) {
      tempHighAlarm = tempHighAlarm * -1;
    }
}

void settingsWrite(){
  // write all settings to the EEPROM for persistance
    for (int i = 0; i < SETTING_SIZE; i++) {
        EEPROM.put(i, settings[i]);
    }
}

void settingsRead(){
    // read settings from EEPROM
    for (int i = 0; i < SETTING_SIZE; i++) {
    EEPROM.get(i, settings[i]);
    }
    // do only once in setup
    // make sure to handle errors
  convertAlarms();
}



void floatDetect() {
    // If current is flowing out or the voltage is too low
    if (current >= 0.8 || voltage < floatVoltage) {
        floating = false; // The battery is not floating
        maxVoltage = 0; // Reset the max voltage
        floatSeconds = 0; // Reset the float timer
    }

    if (voltage > maxVoltage) {
        maxVoltage = voltage;
    }
    
    if (maxVoltage >= chargeVoltage && voltage >= floatVoltage && voltage < 14.0 && current < 0.5 && current > -0.5) {
        if (floatSeconds >= floatSecondsMax) {
            floating = true;
            ampHourFloat = 0;
        }
        else {
            if (refresh) {
                floatSeconds++;
            }
        }
    }
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
            if (enterAddress == &backlightLevel){
                setBack();
            }
            else {
                lcd.clear();
            }
        break;
    }
    // varAtAddress = (varAtAddress + 1) % globalModulo

    // varAtAddress and globalModulo are set when entering each screen
    // make function take an input of which button was pressed.
}

void flashLed() {
    if (flashState) {
        setLed(1);
    }
    else {
        setLed(0);
    }
}

void flashBuzzer() {
    if (flashState) {
        digitalWrite(buzzerPin, HIGH);
    }
    else {
        digitalWrite(buzzerPin, LOW);
    }
}

void checkSettings() {
    if (tempLowAlarm >= tempHighAlarm || voltLowAlarm >= voltHighAlarm) {
        lcd.clear();
        screenHeader(F("SETTINGS ERROR"));
        delay(1000);
        lcd.clear();
        exitSetup = false;
    }
}




void charging() {
    
    chargeHigh = digitalRead(chargePin);
    if (current < -1.0 || chargeHigh) {
        isCharging = true;
    }

    else {
        isCharging = false;
    }
}



