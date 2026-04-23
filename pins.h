#ifndef PINS_H
#define PINS_H

// RELEY X2 BOARD PINS
#define OPTO_PIN 16 // >> DEVBOARD RELAY1 (Outermost) AXT Power supply (PS_ON (Green) TO) PSU GROUND 
#define STATUS_LED_PIN 2 // >> OPTIONAL STATUS (LED TO) EPS32 GROUND 
#define POWER_LED_PIN 23 // >> POWER BUTTON (LED TO) EPS32 GROUND 
#define BUTTON_PIN 22 // >> MOMENTARY POWER (BUTTON TO) EPS32 GROUND 
#define PC_MONITOR_PIN 4 // >> TO BC-250 TPMS1 PIN9 3V 
#define EXTRA_PIN 17 // DEVBOARD RELAY2 (BC-250 POWER BUTTON PIN TO) TPMS1 PIN 17

// BC-250 POWER BUTTON PIN
//        P R
//  0    +↓ ↓  0            
//    o o o o 
//      o o 
//  0   o o   0
//           
//  P = Power PIN
//  R = Reset PIN

// BC-250 TPMS1 PINOUT
//  PCICLK -- [  1   2 ] -- GND
//   FRAME -- [  3   4 ] -- SMB_CLK_MAIN
// PCIRST# -- [  5   6 ] -- SMB_DATA_MAIN
//    LAD3 -- [  7   8 ] -- LAD2
//      3V -- [  9  10 ] -- LAD1
//    LAD0 -- [ 11  12 ] -- GND
//            [     14 ] -- S_PWRDWN#
//    3VSB -- [ 15  16 ] -- SERIRQ#
//     GND -- [ 17  18 ] -- GND


#endif