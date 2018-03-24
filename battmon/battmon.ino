

//libraries that are includes

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#include <SoftI2C.h>
#include <DS3232RTC.h>
#include <avr/pgmspace.h>
#include <string.h>


SoftI2C i2c(A4, A5);
DS3232RTC rtc(i2c);



// set up all the one wire bullshit ( I DO NOT LIKE)

//this tells the onewire library which pin I am using
#define ONE_WIRE_BUS 13 //change to 8 for the real thing

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);




/*

Pinouts as follows

A0 = Buttons
A1 = Current Sensor Output
A2 = Current Sensor Reference
A3 = Charge Signal ----------------------------swap to voltage detect and remove zenner
A4 = Voltage Detect ------------------------------ use as clock input, remove resistors 
A5 = ----------------------------------------------other clock input


D0  =
D1  = LED output
D2  = LCD 7
D3  = LCD 6
D4  = LCD 5
D5  = LCD 4
D6  =
D7  = Load Relay
D8  = Temp Sensor
D9  = Buzzer
D10  = LCD Backlight
D11  = LCD E
D12  = LCD RS
D13  =

Voltage Divider for A4 to read maximum 30VDC
R1 = 500K
R2 = 100K


TODO LIST

- temperature readings -- DONE
- set up the alarms -- DONE
- charge percent indicator -- Replaced with an aH readout
- set up the screens properly --DONe
- set up the buzzer output -- Done
- set up the LED output --Done
- work outhow to make the alarm go for 10 seconds -- Done
- change the tone of the buzzer -- the buzzer i use doesnt do this
- further investigate arrow bug -- Looks pretty good so far
- write all the setup screens
- come up with a percentage reading
- pecentage alarm
- finish off the high and low alarm setup screens


- set up the main screen to say TESTING and DONE over the temp display while in testing mode
- set ssrPin high on setup && when test mode is off
-


Charging logic

battery charger on & current is -ve = up up

battery charger on & current is +ve = x dwn

battery charger off and current -ve = X up

battery charger off and current +ve = dwn dwn


Components

**R1 = 10k Atmega Reset
**R2 = 5k Trim pot or 2
**R3 = 220R Resistor for LCD
R4 = 470k + Voltage input
**R5 = 100k - voltage input
**R6 = 4.7k Temp Probe
*R7 = 470k + Charger input
**R8 = 100k - Charger input
**R9 = 330R LED out 
**R14 = 0R Buzzer
**R15 = 1k Button 1
*R16 = 10k Button 2
*
*


saving stuff in ram notes
have to read the ram on startup, if there is no data then write defaults.



Noted issues

+ve hole on power input too small


*/



// board and variable initialization


//LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //for first run pcb
//LiquidCrystal lcd(2, 3, 4, 5, 11, 12); // for next run PCB (I think)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //this is the test unit LCD


// set up the analog pins
int buttonPin = A0;
int outputPin = A1;
int referencePin = A2;
int chargePin = A3;
int voltagePin = A4;


// set up the digital pins

int ledPin = 1;
int backlightPin = 3; //10 for my PCB 3 for test rig
int buzzPin = 2; //change to 9 when ready
int ssrPin = 11;  //uncomment when ready to test may have to change from 7 for the test rig



// set up the current variables
int outputRaw = 0;
int referenceRaw = 0;
float outputVolts = 0;
float referenceVolts = 0;
float current = 0;

// number of readings for doing smoothing
const int numReadings = 100;

// current averaging
float currentReadings[numReadings];
float currentAverage = 0;
float currentTotal = 0;
int currentIndex = 0;

// set up the voltage variables
int voltsRaw = 0;
float volts = 0;
float kFactor = 0.99236;

// set up the voltage averaging
float voltReadings[numReadings];
float voltAverage = 0;
float voltTotal = 0;
int voltIndex = 0;

// set up the button stuff
int buttonState = LOW;
int buttonTEMP = LOW;
int buttonWas = 1023;
int pressDetect = LOW;

const int enterLOW = -20;
const int enterHIGH = 50;
const int nextLOW = 300;
const int nextHIGH = 600;

long timeSince = 0;
const int pressDelay = 30;

int enterCount = 0;
int nextCount = 0;

const int ENTER = 1;
const int NEXT = 2;


// set up the screens

int resetDelay = 8000;
const int refreshDelay = 1000;
const int flashDelay = 1000;
const int curseDelay = 200;
int flashState = 0;
int curseState = 0;
const int amphourDelay = 250;

unsigned long lastRefresh = 0;
unsigned long screenTime = 0;
unsigned long flashMillis = 0;
unsigned long curseMillis = 0;

int backlightCount = 0;
int capSelect = 0;
int setCount = 0;
int setSelect = 0;
int numScreens = 6;

// doin the amp hour stuff

//long capacity = 8 * 180;
float ampHours = 0;
unsigned long lastAH = 0;
//float percentAH = 100;
int second = 0;
int secondWas = 0;

// Set up the alarm limits

int tempLow = -3;
int tempHigh = 5;




int tAlarmHI = 5;
int tAlarmLO = -2;
int tAlarmON = 1;
int tAlarming = 0;

float avoltAlarm = 11.9;
float vAlarm = 11.9;
int vAlarmON = 1;
int vAlarming = 0;
int vSwitch = 0;



int ledAlarming = 0;
int buzzAlarming = 0;

float tempReading = 1.51;
unsigned long tempMillis = 0;
long tempDelay = 10000;

int charging = 0;
int chargeRaw = 0;
int chargeSig = 0;


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


int lowPos = 0;
int lowTens = 0;
int lowOnes = 0;

int highPos = 1;
int highTens = 0;
int highOnes = 5;

int voltTens = 1;
int voltOnes = 1;
int voltPoint = 9;

int tempAlarm = 3;
int voltAlarm = 1;
int alarmType = 1;


//  battery capacity tester setup section

int mode = 0;
int testDone = 0;
int testWas = 0;
int ssrState = 1;
int pauseState = 0;
float ampHoursTested = 0.0;
float lastAmpHours = 0.0;
int saveAH = 0;

void setup() {





  // set up the LCD's number of columns and rows:
  lcd.createChar(2, ahSymbol);
  lcd.begin(16, 2);


  //setup the input pins as inputs
  pinMode(buttonPin, INPUT);
  pinMode(outputPin, INPUT);
  pinMode(referencePin, INPUT);
  pinMode(chargePin, INPUT);
  pinMode(voltagePin, INPUT);

  //Uncomment for test unit
  analogWrite(voltagePin, 200);
  analogWrite(outputPin, 100);
  analogWrite(referencePin, 50);
  analogWrite(chargePin, HIGH);

  digitalWrite(buttonPin, INPUT_PULLUP);
  //digitalWrite(chargePin, INPUT_PULLUP);
  digitalWrite(ledPin, LOW);
  digitalWrite(ssrPin, HIGH);
  
  
  // setup the output pins as outputs
  pinMode(backlightPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(ssrPin , OUTPUT);


  // sets the smoothing readings to 0
  // this steps through the current reading list and sets all values to 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    currentReadings[thisReading] = 0;
    voltReadings[thisReading] = 0;
  }
  
  

//persistant settings

//

  lowPos = EEPROM.read(10);
  lowTens = EEPROM.read(11);
  lowOnes = EEPROM.read(12);

 highPos = EEPROM.read(13);
 highTens = EEPROM.read(14);
 highOnes = EEPROM.read(15);

  voltTens = EEPROM.read(16);
 voltOnes = EEPROM.read(17);
 voltPoint = EEPROM.read(18);

 tempAlarm = EEPROM.read(19);
 voltAlarm = EEPROM.read(20);
 alarmType = EEPROM.read(21);


 //check the readings from the eeprom

  if (lowPos > 1) {
    lowPos = 0;
  }

  if (lowOnes > 9) {
    lowOnes = 1;
  }

  if (lowTens > 9) {
    lowTens = 2;
  }

   if (highPos > 1) {
    highPos = 1;
  }

  if (highOnes > 9) {
    highOnes = 1;
  }

  if (highTens > 9) {
    highTens = 2;
  }

  if (voltOnes > 9) {
    voltOnes = 1;
  }

  if (voltTens > 9) {
    voltTens = 2;
  }

  if (voltPoint > 9) {
    voltPoint = 0;
  }

  if (tempAlarm > 3) {
    tempAlarm = 0;
  }

  if (voltAlarm > 3) {
    voltAlarm = 1;
  }

  if (alarmType > 3) {
    alarmType = 0;
  }
 
}













void loop() {


   //check the readings from the eeprom

  if (lowPos > 1) {
    lowPos = 0;
  }

  if (lowOnes > 9) {
    lowOnes = 1;
  }

  if (lowTens > 9) {
    lowTens = 2;
  }

   if (highPos > 1) {
    highPos = 1;
  }

  if (highOnes > 9) {
    highOnes = 1;
  }

  if (highTens > 9) {
    highTens = 2;
  }

  if (voltOnes > 9) {
    voltOnes = 1;
  }

  if (voltTens > 9) {
    voltTens = 2;
  }

  if (voltPoint > 9) {
    voltPoint = 0;
  }

  if (tempAlarm > 3) {
    tempAlarm = 0;
  }

  if (voltAlarm > 3) {
    voltAlarm = 1;
  }

  if (alarmType > 3) {
    alarmType = 0;
  }



  // this is a count which is used to time everything

  unsigned long currentMillis = millis();

  
  
  
  
  
  // current readings and calculation------------------------------------------------------------------------------------------------
  
  outputRaw = analogRead(outputPin);
  referenceRaw = analogRead(referencePin);

  outputVolts = outputRaw * (5.0 / 1023.0);
  referenceVolts = referenceRaw * (5.0 / 1023.0);

  current = 160 * (outputVolts - referenceVolts);  

  // smooth out the current readings
  currentTotal -= currentReadings[currentIndex];
  currentReadings[currentIndex] = current;
  currentTotal += currentReadings[currentIndex];
  currentIndex += 1;

  if (currentIndex >= numReadings) {
    currentIndex = 0;
    
  }
 // currentAverage = currentTotal / numReadings; //this is the actual code

  
  if (mode == 0) {
  currentAverage = 60; //use this for testing AH counter

  }

  if ((mode == 1) && (ssrState == 1)) {
    currentAverage = 60;
  }

  else if ((mode == 1) && (ssrState == 0)) {
    currentAverage = 0;
  }

  
  delay(1);

  
  
  
  
  
  
  
  // voltage reading and calculation----------------------------------------------------------------------------------------------

  voltsRaw = analogRead(voltagePin);

// for testing change 5 to 30 when ready to use
 // volts = voltsRaw * (28.5 / 1023.0);
  
  // smooth out the voltage readings

  voltTotal -= voltReadings[voltIndex];
  voltReadings[voltIndex] = voltsRaw;
  voltTotal += voltReadings[voltIndex];
  voltIndex += 1;

  if (voltIndex >= numReadings) {
    voltIndex = 0;
    
  }
  voltAverage = ((voltTotal / numReadings) * kFactor) * (28.5 / 1023.0);
  delay(1);
  

// temperature readings

  if (((currentMillis - tempMillis) > tempDelay) && (mode == 0)) {
    tempMillis = currentMillis;

    sensors.requestTemperatures();
    tempReading = sensors.getTempCByIndex(0);
  }
  
  
  
  
  
  
  
  // reading the buttons--------------------------------------------------------------------------------------------------------------

  pressDetect = analogRead(buttonPin);

  if (pressDetect > enterLOW && pressDetect < enterHIGH) {
    buttonTEMP = 2; //ENTER
    
  }
  else if (pressDetect > nextLOW && pressDetect < nextHIGH) {
    buttonTEMP = 1; //NEXT
    
  }
  else {
    buttonTEMP = 0; //LOW
  }

  if (buttonTEMP != buttonWas) {
    timeSince = millis();
  }

  if ((currentMillis - timeSince) > pressDelay) {

    
    
    if (buttonState != buttonWas) {
      switch (buttonState) {
        case 1: //NEXT
          if (nextCount < 6) {
            nextCount =  ((nextCount +1) % numScreens);
            lcd.clear();
            screenTime = currentMillis;
          }
          else {
            lcd.clear();
            setCount = ((setCount + 1) % 15);
          }
          break;
        
        case 2: //ENTER
          screenTime = currentMillis;
          switch (nextCount) {
            case 0:
                if (mode == 0) {
              backlightCount = ((backlightCount + 1) % 3);
                }

                else {
                  ssrState = ((ssrState + 1) % 2);
                  pauseState += 1;
                  saveAH = 0;
                  //ampHours = 0.0;
                }
              break;
            case 1:
              tempAlarm = ((tempAlarm + 1) % 4);
              EEPROM.write(19, tempAlarm);
              
              break;
            case 2:
              voltAlarm = ((voltAlarm + 1) % 4);
              EEPROM.write(20, voltAlarm);
              
              break;
            case 3:
              alarmType = ((alarmType + 1) % 4);
              EEPROM.write(21, alarmType);
              
              break;
              
            case 4:
              capSelect = ((capSelect + 1) % 2);
              break;
            case 5:
              setSelect = ((setSelect + 1) % 2);
              break;
            case 6:
              switch (setCount) {
                 case 0:
                  lowPos = ((lowPos + 1) % 2);
                   EEPROM.write(10, lowPos);
                  break;
                case 1:
                  lowTens = ((lowTens + 1) % 10);
                   EEPROM.write(11, lowTens);
                  break;
                case 2:
                  lowOnes = ((lowOnes + 1) % 10);
                  EEPROM.write(12, lowOnes);
                  break;
                case 3:
                  highPos = ((highPos + 1) % 2);
                  EEPROM.write(13, highPos);
                  break;
                case 4:
                  highTens = ((highTens + 1) % 10);
                  EEPROM.write(14, highTens);
                  break;
                case 5:
                  highOnes = ((highOnes + 1) % 10);
                  EEPROM.write(15, highOnes);
                  break;
                case 6:
                  voltTens = ((voltTens + 1) % 3);
                  EEPROM.write(16, voltTens);
                  break;
                case 7:
                  voltOnes = ((voltOnes + 1) % 10);
                  EEPROM.write(17, voltOnes);
                  break;
                case 8:
                  voltPoint = ((voltPoint + 1) % 10);
                  EEPROM.write(18, voltPoint);
                  break;
                case 9:
                  mode = ((mode + 1) % 2);
                  if (mode == 1) {
                  EEPROM.get(25, lastAmpHours);
                  }
                  break;
                case 10:
                  setSelect = ((setSelect + 1) % 2);
                  break;
                
                
                
              }
              break;
            case 7:
              
              break;     
          }
          
      }
      buttonState = buttonTEMP;
    }
  }

  buttonWas = buttonTEMP;
  
  
  
 
 



 // combining the numbers

  tempLow = lowOnes + (lowTens * 10);

  if (lowPos == 0) {
    tempLow = tempLow * -1;
  }


  tempHigh = highOnes + (highTens * 10);

  if (highPos == 0) {
    tempHigh = tempHigh * -1;
  }


  avoltAlarm = voltOnes + (voltTens * 10) + (voltPoint * 0.1);
 
 
 
 
 
  // writing shit to the LCD------------------------------------------------------------------------------------------------------------
  
  switch (nextCount) {
    
    // This is the main screen. It is basically a multimeter
    
    case 0:

      setCount = 0;
      setSelect = 0;
      numScreens = 6;
      resetDelay = 8000;
      lcd.noCursor();

      if ((testDone == 0) && (mode == 1) && (pauseState == 0)) {
        ampHours = 0.0;
        
      }
    
      if (capSelect == 1) {
        ampHours = 0;
        capSelect = 0;
      }  
  
      if (vAlarmON == 1 && vAlarming == 1) {
        switch (flashState) {
          case 0:
            lcd.setCursor(0, 0);
            lcd.print(F("V:"));
            break;
          case 1:
            lcd.setCursor(0, 0);
            lcd.print(F("  "));
            break;
        }
      }
    else {
      lcd.setCursor(0, 0);
      lcd.print(F("V:"));
    }
    

    lcd.setCursor(0, 1);
    lcd.print(F("A:"));
    
    lcd.setCursor(8, 1);
    lcd.write(byte(2));
    lcd.print(F(":"));

    
    //lcd.setCursor(10, 1);
      //lcd.print("    ");
      lcd.setCursor(10, 1);
      lcd.print(ampHours, 1);
    
    
    
    /* use this for diagnosis
    lcd.setCursor(10, 1);
    lcd.print(ledAlarming);
    //lcd.print("aH");
    */
    
    
    
    

    switch (mode) {
    case 0:
      if (tAlarmON == 1 && tAlarming == 1) {
          switch (flashState) {
            case 0:
              lcd.setCursor(8, 0);
              lcd.print(F("T:"));
              break;
            case 1:
              lcd.setCursor(8, 0);
              lcd.print(F("  "));
              break;
          }
        }
      else {
        lcd.setCursor(8, 0);
        lcd.print(F("T:"));
      }
      break;

    case 1:
      if ((testDone == 0) && (ssrState == 0)) {
        lcd.setCursor(7, 0);
        
        if (pauseState == 0) {
          lcd.print(F(" Ready  "));
          }

        else {
          lcd.print(F(" Pause  "));
        }
        }

      else if ((testDone == 0) && (ssrState == 1)) {
        lcd.setCursor(7, 0);
        lcd.print(F(" Testing"));
        }

      else {
        lcd.setCursor(7, 0);
        lcd.print(F("--Done--"));
      }
   break;
    }
  
    if ((currentMillis - lastRefresh) > refreshDelay) {
    

      
      lcd.setCursor(2, 0);
      lcd.print(F("     "));
      lcd.setCursor(2, 0);
      lcd.print(voltAverage, 1);

      lcd.setCursor(2, 1);
      lcd.print(F("      "));
      lcd.setCursor(2, 1);
      lcd.print(currentAverage, 1);
      
      if (mode == 0) {
        lcd.setCursor(10, 0);
        lcd.print(F("      "));
        lcd.setCursor(10, 0);
        lcd.print(tempReading, 1);
        lcd.print((char)223);
      }
      //lcd.setCursor(10, 1);
      //lcd.print("    ");
      //lcd.setCursor(10, 1);
      //lcd.print(ampHours, 0);
      //lcd.print(chargeRaw);
      //lcd.setCursor(11, 1);
      //lcd.print("     ");
      //lcd.setCursor(8, 1);
      //lcd.print(pressDetect);
      //lcd.print(percentAH, 0);
      //lcd.print("%");

       //logic for the charging arrows

       //I have decided I don't like the arrows
       
     /* if (charging == 1) {
        lcd.setCursor(15, 1);
        lcd.write(byte(0));
      }

      else {
        lcd.setCursor(15, 1);
        lcd.write("-");
      }

      
      else if (charging == 1 && currentAverage > 0.5) {
        lcd.setCursor(14, 1);
        lcd.write(byte(0));
        lcd.write(byte(1));
      }
      else if (charging == 1 && currentAverage < -0.5) {
        lcd.setCursor(14, 1);
        lcd.write(byte(1));
        lcd.print("x");
      }
      else if (charging == 0 && currentAverage < -0.5) {
        lcd.setCursor(14, 1);
        lcd.print("x");
        lcd.write(byte(0));
      }
    
      else if (charging == 0 && currentAverage > 0.5) {
        lcd.setCursor(14, 1);
        lcd.write(byte(1));
        lcd.write(byte(1));
      }
      else {
        lcd.setCursor(14, 1);
        lcd.print("--");
      
      }

      */
      
      
      lastRefresh = currentMillis;
    }
  break;
  
  
  
  
  
  // this is the tempreture alarm setup screen
  
  case 1:
  lcd.setCursor(0, 0);
  lcd.print(F("Temp Alarm Limit"));
  
  switch (tempAlarm) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(F("Fridge -2"));
      lcd.print((char)223);
      lcd.print(F(" to 5"));
      lcd.print((char)223);
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(F("Freeze-50"));
      lcd.print((char)223);
      lcd.print(F("to-15"));
      lcd.print((char)223);
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print(F("User "));
      lcd.print(tempLow);
      lcd.print((char)223);
      lcd.print(F(" to "));
      lcd.print(tempHigh);
      lcd.print((char)223);
      lcd.print(F("    "));
      break;
    case 3:
      ledAlarming = 0;
      buzzAlarming = 0;
      lcd.setCursor(0, 1);
      lcd.print(F("Off             "));
      break;  
  }
  break;
  
  
  
  
  
  
  //This is the voltage alarm setup screen
  
  case 2:
  lcd.setCursor(0, 0);
  lcd.print(F("Volt alarm level"));
  
  if (vAlarmON == 0) {
    ledAlarming = 0;
    buzzAlarming = 0;
    lcd.setCursor(0, 1);
    lcd.print(F("Off             "));
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print(vAlarm, 1);
    lcd.print(F(" Volts "));
    switch (voltAlarm) {
      case 2:
      lcd.setCursor(12, 1);
      lcd.print(F("User"));
      break;
        
    }
  }
  
  
  
  break;
  
  
  
  
  
  
  // this is the alarm option screen
  
  case 3:
  lcd.setCursor(0, 0);
  lcd.print(F("Which Alarm?"));
  switch (alarmType) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(F("LED & Buzzer    "));
      break;
    case 1:
      
      buzzAlarming = 0;
      lcd.setCursor(0, 1);
      lcd.print(F("LED Only        "));
      break;
    case 2:
      ledAlarming = 0;
      lcd.setCursor(0, 1);
      lcd.print(F("Buzzer Only     "));
      break;
    case 3:
    buzzAlarming = 0;
    ledAlarming = 0;
      lcd.setCursor(0, 1);
      lcd.print(F("None            "));
      break;  
  }
  
  break;
  
  
  // aH reset screen
  
  
  case 4:
  lcd.setCursor(0, 0);
  lcd.print(F("Reset Amp Hours?"));
  switch (capSelect) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(F("No              "));
      
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(F("Yes             "));
      break;
  }
  
  break;






//setup screens


  case 5:
  lcd.setCursor(0, 0);
  lcd.print(F("Enter Setup?"));
  switch (setSelect) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(F("No              "));
      
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(F("Yes             "));
      numScreens = 10;
      break;
  }
  break;
  case 6:
   switch (setCount) {
    
    
    
    
    
    // set the low alarm
    
    case 0:
      resetDelay = 30000;
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Temp Low Alarm"));
      lcd.setCursor(0, 1);
      if (curseState == 1 && lowPos == 1) {
        lcd.print(F("+"));
      }
      else if (curseState == 1 && lowPos == 0) {
        lcd.print(F("-"));
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print(lowTens);
      lcd.print(lowOnes);
      lcd.print((char)223);
      lcd.print(F("C"));
      break;
    case 1:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Temp Low Alarm"));
      lcd.setCursor(0, 1);
      if (lowPos == 1) {
        lcd.print(F("+"));
      }
      else {
        lcd.print("-");
      }
      if (curseState == 1) {
        lcd.print(lowTens);
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print(lowOnes);
      lcd.print((char)223);
      lcd.print("C");
      break;
    case 2:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Temp Low Alarm"));
      lcd.setCursor(0, 1);
      if (lowPos == 1) {
        lcd.print(F("+"));
      }
      else {
        lcd.print(F("-"));
      }
      lcd.print(lowTens);
      if (curseState == 1) {
        lcd.print(lowOnes);
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print((char)223);
      lcd.print(F("C"));
      break;
    
    
    
    
    
    
    
    
    // set the high alarm
    
    
    
    
    
    
    
    
    case 3:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Temp High Alarm"));
      lcd.setCursor(0, 1);
      if (curseState == 1 && highPos == 1) {
        lcd.print(F("+"));
      }
      else if (curseState == 1 && highPos == 0) {
        lcd.print(F("-"));
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print(highTens);
      lcd.print(highOnes);
      lcd.print((char)223);
      lcd.print(F("C"));
      break;
    case 4:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Temp High Alarm"));
      lcd.setCursor(0, 1);
      if (highPos == 1) {
        lcd.print(F("+"));
      }
      else {
        lcd.print(F("-"));
      }
      if (curseState == 1) {
        lcd.print(highTens);
      }
      else {
        lcd.print(" ");
      }
      lcd.print(highOnes);
      lcd.print((char)223);
      lcd.print(F("C"));
      break;
    case 5:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Temp High Alarm"));
      lcd.setCursor(0, 1);
      if (highPos == 1) {
        lcd.print(F("+"));
      }
      else {
        lcd.print(F("-"));
      }
      lcd.print(highTens);
      if (curseState == 1) {
        lcd.print(highOnes);
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print((char)223);
      lcd.print(F("C"));
      break;

    
    
    case 6:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Volt Low Alarm"));
      lcd.setCursor(0, 1);
      if (curseState == 1) {
        lcd.print(voltTens);
      }
      else {
        lcd.print(" ");
      }
      lcd.print(voltOnes);
      lcd.print(F("."));
      lcd.print(voltPoint);
      lcd.print(F("Volts"));
      break;
    case 7:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Volt Low Alarm"));
      lcd.setCursor(0, 1);
      lcd.print(voltTens);
      if (curseState == 1) {
        lcd.print(voltOnes);
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print(F("."));
      lcd.print(voltPoint);
      lcd.print(F("Volts"));
      break;
    case 8:
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(F("Volt Low Alarm"));
      lcd.setCursor(0, 1);
      lcd.print(voltTens);
      lcd.print(voltOnes);
      lcd.print(F("."));
      if (curseState == 1) {
        lcd.print(voltPoint);
      }
      else {
        lcd.print(F(" "));
      }
      lcd.print(F("Volts"));
      break;
    
    
    case 9:
      lcd.setCursor(0, 0);
      lcd.print(F("Batt Test Mode"));
      switch (mode) {
        case 0:
          lcd.setCursor(0, 1);
          lcd.print(F("Off             "));
          testDone = 6;
          break;
        case 1:            
          lcd.setCursor(0, 1);
          lcd.print(F("On "));
          lcd.print(F("Last:"));
          lcd.print(lastAmpHours, 1);
          lcd.print(F("Ah"));
          ssrState = 0;
          testDone = 0;
          pauseState = 0;
          
          break;
      }
      break;
    case 10:
      lcd.setCursor(0, 0);
      lcd.print(F("Exit Setup?"));
        switch (setSelect) {
          case 0:
            lcd.setCursor(0, 1);
            lcd.print(F("Yes              "));
      
          break;
          case 1:
            lcd.setCursor(0, 1);
            lcd.print(F("No             "));
          break;
  }
      
      break;
    case 11:
      if (setSelect == 0) {
        nextCount = 0;
      }

      else {
        setCount = 0;
      }
      break;
   }





    
  break;
  
 /* case 7:
    lcd.setCursor(0, 0);
    lcd.print("still nope");
  break;
   
  case 8:
    lcd.setCursor(0, 0);
    lcd.print("yeah....");
  break;
  
  case 9:
    lcd.setCursor(0, 0);
    lcd.print("I am yet to write");
    lcd.setCursor(0, 1);
    lcd.print("    These screens");
  break;

  */
  }

  
  
  
  
  
 
      
  


  
// changing the options;---------------------------------------------------------------------------------------------------------------------













// the backlight
  
  switch (backlightCount) {
    case 0:
      analogWrite(backlightPin, 255);
      break;
    case 1:
      analogWrite(backlightPin, 50);
      break;
    case 2:
      analogWrite(backlightPin, 0);
      break;
    //case 3:  //reduced backlight to 3options
      //analogWrite(backlightPin, 0);
      //break;
  }   






















// Temp alarm levels

  switch (tempAlarm) {
    case 0:
      tAlarmHI = 5;
      tAlarmLO = -2;
      tAlarmON = 1;
      break;
    case 1:
      tAlarmHI = -15;
      tAlarmLO = -50;
      tAlarmON = 1;
      break;
    case 2:
      tAlarmHI = tempHigh;
      tAlarmLO = tempLow;
      tAlarmON = 1;
      break;
    case 3:
      tAlarmON = 0;
      break;  
  }
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  // volt alarm levels
  
  switch (voltAlarm) {
    case 0:
      vAlarm = 11.5;
      vAlarmON = 1;
      break;
    case 1:
      vAlarm = 11.9;
      vAlarmON = 1;
      break;
    case 2:
      vAlarm = avoltAlarm;
      vAlarmON = 1;
      break;
    case 3:
      vAlarmON = 0;
      break;
    
  }
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  // sets when the alarms should go off ---------------------------------------------------------------
  
  if ((voltAverage < vAlarm) && (mode == 0)) {
    vAlarming = 1;
  }
  else {
    vAlarming = 0;
  }
  
  if ((tempReading > tAlarmHI || tempReading < tAlarmLO) && (mode == 0)) {
    tAlarming = 1;
  }
  else {
    tAlarming = 0;
  }
  
  
// flash the display

if ((currentMillis - flashMillis) > flashDelay) {
  flashState = ((flashState + 1) % 2);
  flashMillis = currentMillis;
}

//flash the cursor

if ((currentMillis - curseMillis) > curseDelay) {
  curseState = ((curseState + 1) % 2);
  curseMillis = currentMillis;
}


// Return to main display after 8 second delay  



if (nextCount > 0) {
  
  if ((currentMillis - screenTime) > resetDelay) {
    lcd.clear();
    nextCount = 0;
    
  }
 }


// check the charging 


  chargeRaw = analogRead(chargePin);


  //chargeSig = chargeRaw;
  
  if (chargeRaw > 550) {
     charging = 1;
  }
  else {
    charging = 0;
  }
  













// coulumb counting - this is experimental and probably not very accurate.


  RTCTime time;

  rtc.readTime(&time);

  second = time.second;
  
  if (second != secondWas) {
    ampHours = ampHours + (currentAverage / 3600);
  }
  secondWas = second;
  
  /* if ((currentMillis - lastAH) > amphourDelay) {
    ampHours = ampHours + (currentAverage / 14400);
    //ampHours += 1;
    lastAH = currentMillis;
  }
  */
  
  if (ampHours < 0) {
    ampHours = 0;
  }
  











// LED Control

// ledPin, vAlarming, tAlarming, alarmType 0 or 1 for LED

  if (((vAlarming == 1 && vAlarmON == 1) || (tAlarming == 1 && tAlarmON == 1)) && (alarmType == 0 || alarmType == 1)){
    
    ledAlarming = 1500;
    
    
        
}
 

 

  if (ledAlarming > 0) {
     ledAlarming -= 1;
     switch (flashState) {
            case 0:
              digitalWrite(ledPin, LOW);
              break;
            case 1:
              digitalWrite(ledPin, HIGH);
              break;
          }
  }
  else {  
    if (charging == 1) {
      digitalWrite(ledPin, HIGH);
  }
    else {
      digitalWrite(ledPin, LOW);
    }
  }
  
// buzzer control will go here  

  // buzzPin, vAlarming, tAlarming, alarmType 0 or 2 for buzzer

  if (((vAlarming == 1 && vAlarmON == 1) || (tAlarming == 1 && tAlarmON == 1)) && (alarmType == 0 || alarmType == 2)){
    
    buzzAlarming = 1500;
    
    
        
}
  if (buzzAlarming > 0) {
     buzzAlarming -= 1;
     
     
     switch (flashState) {
            case 0:
              digitalWrite(buzzPin, LOW);
              break;
            case 1:
              digitalWrite(buzzPin, HIGH);
              break;
          }
          
  }
  else {  
   digitalWrite(buzzPin, LOW);
  }

  


  if ((voltAverage < avoltAlarm) && (mode == 1)) {
    testDone += 1;
    saveAH += 1;
  }

  

  if ((saveAH == 1) && (mode == 1)) {
    ampHoursTested = ampHours;
    EEPROM.put(25, ampHoursTested);
    saveAH += 1;
    if (saveAH > 10) {
      saveAH = 10;
    }
    
    
  }

  testWas = testDone;
  
 if (testDone > 0) {
  ssrState = 0;
 }


  if (ssrState == 0) {
    digitalWrite(ssrPin, LOW);
  }
  else {
    digitalWrite(ssrPin, HIGH);
  }

  


 
  
}

  
