EESchema Schematic File Version 2
LIBS:ac-dc
LIBS:adc-dac
LIBS:Altera
LIBS:analog_devices
LIBS:analog_switches
LIBS:atmel
LIBS:audio
LIBS:Battery_Management
LIBS:bbd
LIBS:Bosch
LIBS:brooktre
LIBS:Connector
LIBS:contrib
LIBS:cypress
LIBS:dc-dc
LIBS:Decawave
LIBS:device
LIBS:digital-audio
LIBS:Diode
LIBS:Display
LIBS:driver_gate
LIBS:dsp
LIBS:DSP_Microchip_DSPIC33
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:Espressif
LIBS:FPGA_Actel
LIBS:ftdi
LIBS:gennum
LIBS:Graphic
LIBS:hc11
LIBS:infineon
LIBS:intel
LIBS:interface
LIBS:intersil
LIBS:ir
LIBS:Lattice
LIBS:LED
LIBS:LEM
LIBS:linear
LIBS:Logic_74xgxx
LIBS:Logic_74xx
LIBS:Logic_CMOS_4000
LIBS:Logic_CMOS_IEEE
LIBS:logic_programmable
LIBS:Logic_TTL_IEEE
LIBS:maxim
LIBS:MCU_Microchip_PIC10
LIBS:MCU_Microchip_PIC12
LIBS:MCU_Microchip_PIC16
LIBS:MCU_Microchip_PIC18
LIBS:MCU_Microchip_PIC24
LIBS:MCU_Microchip_PIC32
LIBS:MCU_NXP_Kinetis
LIBS:MCU_NXP_LPC
LIBS:MCU_NXP_S08
LIBS:MCU_Parallax
LIBS:MCU_ST_STM8
LIBS:MCU_ST_STM32
LIBS:MCU_Texas_MSP430
LIBS:Mechanical
LIBS:memory
LIBS:microchip
LIBS:microcontrollers
LIBS:modules
LIBS:Motor
LIBS:motor_drivers
LIBS:motorola
LIBS:nordicsemi
LIBS:nxp
LIBS:onsemi
LIBS:opto
LIBS:Oscillators
LIBS:philips
LIBS:power
LIBS:powerint
LIBS:Power_Management
LIBS:pspice
LIBS:references
LIBS:regul
LIBS:Relay
LIBS:RF_Bluetooth
LIBS:rfcom
LIBS:RFSolutions
LIBS:Sensor_Current
LIBS:Sensor_Humidity
LIBS:sensors
LIBS:silabs
LIBS:siliconi
LIBS:supertex
LIBS:Switch
LIBS:texas
LIBS:Transformer
LIBS:Transistor
LIBS:triac_thyristor
LIBS:Valve
LIBS:video
LIBS:wiznet
LIBS:Worldsemi
LIBS:Xicor
LIBS:xilinx
LIBS:zetex
LIBS:Zilog
LIBS:lemhtfs
LIBS:1503_19
LIBS:A-2004-3-4-LP-N-R
LIBS:CurrentSensor-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L LEMHTFS U1
U 1 1 5A477259
P 6700 4000
F 0 "U1" H 6700 3400 60  0000 C CNN
F 1 "LEMHTFS" H 6700 4000 60  0000 C CNN
F 2 "Hall-Effect_Transducers_LEM:HTFS-Series_LEM-HallEffektCurrentTransducer" H 6700 4000 60  0001 C CNN
F 3 "" H 6700 4000 60  0001 C CNN
	1    6700 4000
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 5A4772B0
P 4700 4200
F 0 "C1" H 4725 4300 50  0000 L CNN
F 1 "47nF" H 4725 4100 50  0000 L CNN
F 2 "Capacitors_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm" H 4738 4050 50  0001 C CNN
F 3 "" H 4700 4200 50  0001 C CNN
	1    4700 4200
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 5A47732F
P 5250 4550
F 0 "C3" H 5275 4650 50  0000 L CNN
F 1 "4.7nF" H 5275 4450 50  0000 L CNN
F 2 "Capacitors_THT:C_Disc_D5.0mm_W2.5mm_P2.50mm" H 5288 4400 50  0001 C CNN
F 3 "" H 5250 4550 50  0001 C CNN
	1    5250 4550
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 5A47735E
P 4850 4550
F 0 "C2" H 4875 4650 50  0000 L CNN
F 1 "47nF" H 4875 4450 50  0000 L CNN
F 2 "Capacitors_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm" H 4888 4400 50  0001 C CNN
F 3 "" H 4850 4550 50  0001 C CNN
	1    4850 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 4150 5150 4150
Wire Wire Line
	5150 4150 5150 4050
Wire Wire Line
	5150 4050 2950 4050
Wire Wire Line
	4450 4350 5550 4350
Wire Wire Line
	5550 4350 5550 4250
Wire Wire Line
	5550 4250 6250 4250
Wire Wire Line
	5250 4350 5250 4400
Connection ~ 5250 4350
Wire Wire Line
	2500 4750 5650 4750
Wire Wire Line
	5650 4750 5650 4350
Wire Wire Line
	5650 4350 6250 4350
Wire Wire Line
	5250 4750 5250 4700
Wire Wire Line
	4850 4350 4850 4400
Connection ~ 4850 4350
Wire Wire Line
	4850 4700 4850 4900
Wire Wire Line
	4000 4900 5800 4900
Wire Wire Line
	5800 4900 5800 4450
Wire Wire Line
	5800 4450 6250 4450
$Comp
L A-2004-3-4-LP-N-R J1
U 1 1 5A4774F9
P 2950 4400
F 0 "J1" H 3300 4550 50  0000 C CNN
F 1 "A-2004-3-4-LP-N-R" H 3300 4150 50  0000 C CNN
F 2 "currentsensor:A-2004-3-4-LP-N-R" H 3300 4050 50  0001 C CNN
F 3 "http://www.assmann-wsw.com/fileadmin/datasheets/ASS_7041_CO.pdf" H 3300 3950 50  0001 C CNN
F 4 "CONN MOD JACK 4P4C R/A UNSHLD" H 3300 3850 50  0001 C CNN "Description"
F 5 "RS" H 3300 3750 50  0001 C CNN "Supplier_Name"
F 6 "" H 3300 3650 50  0001 C CNN "RS Part Number"
F 7 "ASSMANN WSW components GmbH" H 3300 3550 50  0001 C CNN "Manufacturer_Name"
F 8 "A-2004-3-4-LP-N-R" H 3300 3450 50  0001 C CNN "Manufacturer_Part_Number"
F 9 "" H 3300 3350 50  0001 C CNN "Allied_Number"
F 10 "" H 3300 3250 50  0001 C CNN "Other Part Number"
F 11 "" H 3500 3150 50  0001 C CNN "Height"
	1    2950 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 4050 2950 4400
Connection ~ 4700 4050
Wire Wire Line
	4450 4350 4450 4500
Wire Wire Line
	4450 4500 3650 4500
Connection ~ 4700 4350
Wire Wire Line
	2500 4750 2500 4500
Wire Wire Line
	2500 4500 2950 4500
Connection ~ 5250 4750
Wire Wire Line
	3650 4400 4000 4400
Wire Wire Line
	4000 4400 4000 4900
Connection ~ 4850 4900
$EndSCHEMATC
