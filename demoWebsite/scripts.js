

console.log("Oh hi there")

const CHARGE_CURRENT = -25;
const FRIDGE_CURRENT = 3.6;
const LIGHTS_CURRENT = 1.5;
const COMPRESSOR_CURRENT = 50;
const CONTIN_CURRENT = 200;

const DISCHARGE_INCREMENT = 0.02;
const CHARGED_VOLTAGE = 12.8;

const TEMP_MAX = 30.5;
const FRIDGE_MAX = 1.5;
const FRIDGE_MIN = 1.1;
const FRIDGE_WARM = 0.05;
const FRIDGE_COOL = -0.1;


var voltReading;
var fullCurrent = 0.0;
var currentReading = 0.0;
var tempReading = 5.5;
var ampHourReading = 0;

var ampHourFloat = 0.0;

var chargeOn = false;
var fridgeOn = false;
var fridgeRunning = false;
var fridgeModifier = 0;
var lightsOn = false;
var compressorOn = false;
var continOn = false;

var voltArea;

var voltElement;
var tempElement;
var currentElement;
var ampHourElement;

var chargeSwitch;
var fridgeSwitch;
var lightsSwitch;
var compressorSwitch;
var continSwitch;

var segments;


var cursor = 0;

var voltAlarmLevel = 11.9;
var tempAlarmLevel = 4.5;

var screenSelect = 0;



window.onload = function() {
    //voltArea = document.getElementsByClassName("voltArea");
    //tempArea = document.getElementsByClassName("tempArea");
    //currentArea = document.getElementsByClassName("currentArea");
    //ampHourArea = document.getElementsByClassName("ampHourArea");

    segments = document.getElementsByClassName("segment");

    chargeSwitch = document.getElementById("chargeSwitch");
    fridgeSwitch = document.getElementById("fridgeSwitch");
    lightsSwitch = document.getElementById("lightsSwitch");
    compressorSwitch = document.getElementById("compressorSwitch");
    continSwitch = document.getElementById("continSwitch");

    enterButton = document.getElementById("enterButton");

    enterButton.onclick = function() {
        clearScreen();
        screenSelect++;
        refreshDisplay();
    }


    chargeSwitch.onclick = function() {
        chargeOn = flickSwitch(chargeOn, CHARGE_CURRENT, chargeSwitch);
    };

    fridgeSwitch.onclick = function() {
        fridgeOn = flickSwitch(fridgeOn, FRIDGE_CURRENT, fridgeSwitch);
    };

    lightsSwitch.onclick = function() {
        lightsOn = flickSwitch(lightsOn, LIGHTS_CURRENT, lightsSwitch);
    };    
    
    compressorSwitch.onclick = function() {
        compressorOn = flickSwitch(compressorOn, COMPRESSOR_CURRENT, compressorSwitch);
    };

    continSwitch.onclick = function() {
        continOn = flickSwitch(continOn, CONTIN_CURRENT, continSwitch);
    };


    //refreshDisplay();
    setInterval(function() {
        calculateCurrent();
        calculateAmpHours();
        calculateVoltage();
        fridgeCycle();
        updateTemps();
        updateCharger();
        refreshDisplay();
    }, 1000);
}

function displayWrite(string) {
    if (typeof string != 'string') {
        string = string.toString();
    };
    


    for (var i = 0; i < string.length; i++, cursor++) {
        segments[cursor].innerHTML = string.charAt(i);
    }
}

function clearScreen() {
    cursor = 0;
    displayWrite("                                ");
    cursor = 0;
}





// ------ DISPLAYS ------



// Multimeter
function mainDisplay() {
    cursor = 0;
    displayWrite("V:");
    displayWrite(voltReading.toFixed(1));

    cursor = 8;
    displayWrite("T:");
    displayWrite(tempReading.toFixed(1) + String.fromCharCode(176));

    cursor = 16;
    displayWrite("A:");
    displayWrite(currentReading.toFixed(1));

    cursor = 24;
    displayWrite("H:");
    displayWrite(ampHourReading.toFixed(0));

}

function refreshDisplay() {

    switch(screenSelect) {
        case 0:
            mainDisplay();
        break;

        case 1:
            voltAlarmDisplay();
        break;

        case 2:
            tempAlarmDisplay();
        break;

        case 3:
            screenSelect = 0;
            refreshDisplay();
        break;

    }
}

// Voltage Alarm Setting
function voltAlarmDisplay() {
    cursor = 0;
    displayWrite("Low Volt Alarm");
    cursor = 16;
    displayWrite(voltAlarmLevel);
    displayWrite("V");
}


// Temp Alarm
function tempAlarmDisplay() {
    cursor = 0;
    displayWrite("Temp High Alarm");
    cursor = 16;
    displayWrite(tempAlarmLevel);
    displayWrite(String.fromCharCode(176));
}


// ------ WEBPAGE SWITCHES ------

function flickSwitch(item, current, whichSwitch) {
    //console.log(whichSwitch);
    if (item) {
        fullCurrent -= current;
        whichSwitch.style.backgroundColor = "red";
        }

    else {
        fullCurrent += current;
        whichSwitch.style.backgroundColor = "green";
    }


    return !item;
}


// ------ CALCULATE VALUES ------
function calculateCurrent() {
     currentReading = fullCurrent + fridgeModifier;
 } 


function calculateAmpHours() {
    ampHourFloat += currentReading / 60;
    if (ampHourFloat < 0) {
        ampHourFloat = 0;
    }

    //console.log(ampHourFloat);

    ampHourReading = Math.floor(ampHourFloat);
}

function calculateVoltage() {
    voltReading = CHARGED_VOLTAGE - (DISCHARGE_INCREMENT * ampHourFloat);

    if (voltReading < 10.5) {
        voltReading = 10.5;
    }

}

function updateCharger() {
    if(chargeOn && ampHourFloat <= 0) {
        chargeOn = flickSwitch(chargeOn, CHARGE_CURRENT, chargeSwitch);
    }
}

function updateTemps() {
    if (fridgeRunning) {
        tempReading += FRIDGE_COOL;
    }

    else {
        tempReading += FRIDGE_WARM;

        if (tempReading > 30.8) {
            tempReading = 30.8;
        }

    }
}








function fridgeCycle() {
    // while fridge is on

    if (fridgeOn) {
        if (tempReading > FRIDGE_MAX) {
            fridgeRunning = true;
            changeBgColour("fridgeSwitch", "green");
            fridgeModifier = 0;


        }

        else if (tempReading < FRIDGE_MIN) {
            fridgeRunning = false;
            changeBgColour("fridgeSwitch", "yellow");
            fridgeModifier = FRIDGE_CURRENT * -1;
            
        }
        
    }

    else {
        fridgeRunning = false;
        fridgeModifier = 0;
        changeBgColour("fridgeSwitch", "red");



    }
}

function changeBgColour(id, colour) {
    var element = document.getElementById(id);

    element.style.backgroundColor = colour;
}