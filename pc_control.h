#ifndef PC_CONTROL_H
#define PC_CONTROL_H

#include "pins.h"
#include <Arduino.h>
#include <Bluepad32.h>

// Power-tilakoneen tilat
enum PowerState {
    POWER_IDLE,
    POWER_ON_START,
    POWER_ON_WAITING_RELAY2,
    POWER_ON_COMPLETE,
    POWER_OFF_START,
    POWER_OFF_WAITING,
    POWER_OFF_WAITING_POWEROFF,
    POWER_FORCE_START,
    POWER_FORCE_WAITING
};

extern bool pcIsOn;
extern bool shutdownRequested;
extern bool forceShutdown;
extern unsigned long forceShutdownStartTime;
extern const unsigned long forceShutdownDuration;

// Filtteröintimuuttujat
extern bool filteredPcState;
extern unsigned long lastPcChangeTime;
extern const unsigned long pcStableDelay;

// Power-tilakoneen muuttujat
extern PowerState powerState;
extern unsigned long powerStateStartTime;

// ================ GLOBAALIT DEBOUNCE-MUUTTUJAT ================
bool debounceLastRaw = false;
unsigned long debounceLastChange = 0;
bool debounceStableState = false;

// ================ DEBOUNCE-FUNKTIO ================
bool debouncePcState(bool rawState) {
    if (rawState != debounceLastRaw) {
        debounceLastRaw = rawState;
        debounceLastChange = millis();
        Serial.print("DEBOUNCE: Raakatila muuttui -> ");
        Serial.println(rawState ? "HIGH" : "LOW");
    }
    
    if (millis() - debounceLastChange >= pcStableDelay) {
        if (debounceStableState != rawState) {
            debounceStableState = rawState;
            Serial.print("DEBOUNCE: Suodatettu tila vakiintui -> ");
            Serial.println(debounceStableState ? "HIGH" : "LOW");
        }
    }
    
    return debounceStableState;
}

// ================ getStablePcState ================
bool getStablePcState() {
    return filteredPcState;
}

void initPins() {
    pinMode(OPTO_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(POWER_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(PC_MONITOR_PIN, INPUT);
    pinMode(EXTRA_PIN, OUTPUT);
    
    bool initial = digitalRead(PC_MONITOR_PIN);
    digitalWrite(OPTO_PIN, initial ? HIGH : LOW);
    digitalWrite(POWER_LED_PIN, initial ? HIGH : LOW);
    digitalWrite(STATUS_LED_PIN, HIGH);
    digitalWrite(EXTRA_PIN, LOW);
    
    // Alusta debounce-muuttujat
    debounceLastRaw = initial;
    debounceStableState = initial;
    filteredPcState = initial;
    
    if (powerState == POWER_IDLE) {
        pcIsOn = initial;
    }
    
    powerState = POWER_IDLE;
}

// ================ KORJATTU updatePcState() ESP.restart() KANSSA ================
void updatePcState() {
    bool currentMonitor = digitalRead(PC_MONITOR_PIN);
    bool newFilteredState = debouncePcState(currentMonitor);
    
    if (newFilteredState != filteredPcState) {
        Serial.print(">>> PC TILAN MUUTOS: ");
        Serial.print(filteredPcState ? "ON" : "OFF");
        Serial.print(" -> ");
        Serial.print(newFilteredState ? "ON" : "OFF");
        Serial.print(" (powerState=");
        Serial.print(powerState);
        Serial.println(")");
        filteredPcState = newFilteredState;
    }
    
    // Käsittele PC:n tilan muutokset
    if (filteredPcState != pcIsOn) {
        
        if (filteredPcState == HIGH) {
            // PC KÄYNNISTYI
            Serial.println("*** PC KÄYNNISTYI ***");
            pcIsOn = true;
            shutdownRequested = false;
            forceShutdown = false;
            
            if (powerState == POWER_IDLE) {
                digitalWrite(OPTO_PIN, HIGH);
                digitalWrite(POWER_LED_PIN, HIGH);
            }
        } else {
            // PC SAMMUI
            Serial.println("*** PC SAMMUI ***");
            pcIsOn = false;
            shutdownRequested = false;
            forceShutdown = false;
            
            if (powerState == POWER_IDLE) {
                digitalWrite(OPTO_PIN, LOW);
                digitalWrite(POWER_LED_PIN, LOW);
                
                // ================ ESP.restart() PC:N SAMMUESSA ================
                // Tämä tyhjentää Bluepad32:n tilat ja mahdollistaa
                // ohjaimen uudelleenhavaitsemisen
                Serial.println("PC sammui - Käynnistetään ESP32 uudelleen 2 sekunnin kuluttua...");
                delay(2000);
                ESP.restart();
            }
        }
    }
}

// KÄYNNISTYS
void startPowerOn() {
    if (filteredPcState == HIGH) {
        Serial.println("PC already on");
        return;
    }
    
    if (powerState != POWER_IDLE) {
        Serial.println("Power operation already in progress");
        return;
    }
    
    Serial.println("=== POWER ON SEQUENCE STARTED ===");
    powerState = POWER_ON_START;
    powerStateStartTime = millis();
}

// NORMAALI SAMMUTUS
void startNormalShutdown() {
    if (filteredPcState == LOW) {
        Serial.println("PC already off");
        return;
    }
    
    if (powerState != POWER_IDLE) {
        Serial.println("Power operation already in progress");
        return;
    }
    
    Serial.println("=== NORMAL SHUTDOWN STARTED ===");
    powerState = POWER_OFF_START;
    powerStateStartTime = millis();
}

// PAKKOSAMMUTUS
void startForceShutdown() {
    if (filteredPcState == LOW) {
        Serial.println("PC already off");
        return;
    }
    
    if (powerState != POWER_IDLE) {
        Serial.println("Power operation already in progress");
        return;
    }
    
    Serial.println("=== FORCE SHUTDOWN STARTED ===");
    powerState = POWER_FORCE_START;
    powerStateStartTime = millis();
}

// POWER-TILOJEN HALLINTA
void handlePowerStates() {
    unsigned long now = millis();
    
    switch (powerState) {
        case POWER_ON_START:
            Serial.println("POWER ON START - Setting relays");
            btStop();
            digitalWrite(OPTO_PIN, HIGH);
            digitalWrite(EXTRA_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, HIGH);
            powerState = POWER_ON_WAITING_RELAY2;
            powerStateStartTime = now;
            break;
        
        case POWER_ON_WAITING_RELAY2:
            if (now - powerStateStartTime >= 1000) {
                Serial.println("RELAY 2: 1s passed - turning EXTRA_PIN OFF");
                digitalWrite(EXTRA_PIN, LOW);
                powerState = POWER_ON_COMPLETE;
                powerStateStartTime = now;
            }
            break;
            
        case POWER_ON_COMPLETE:
            if (now - powerStateStartTime >= 8000) {
                if (filteredPcState == HIGH) {  // KÄYTÄ SUODATETTUA TILAA
                    Serial.println("PC power-on confirmed - PC is now running");
                } else {
                    Serial.println("WARNING: PC did not power on! Turning relay OFF");
                    digitalWrite(OPTO_PIN, LOW);
                    btStart();
                }
                powerState = POWER_IDLE;
            }
            break;
            
        case POWER_OFF_START:
            Serial.println("Normal shutdown - EXTRA_PIN HIGH for 500ms");
            digitalWrite(EXTRA_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, LOW);
            powerState = POWER_OFF_WAITING;
            powerStateStartTime = now;
            break;
            
        case POWER_OFF_WAITING:
            if (now - powerStateStartTime >= 500) {
                Serial.println("Shutdown pulse complete - releasing EXTRA_PIN");
                digitalWrite(EXTRA_PIN, LOW);
                powerState = POWER_OFF_WAITING_POWEROFF;
                powerStateStartTime = now;
            }
            break;
            
        case POWER_OFF_WAITING_POWEROFF:
            // Odotetaan että PC sammuu (filteredPcState menee LOW) JA pysyy siinä 4 sekuntia
            if (filteredPcState == LOW) {  // KÄYTÄ SUODATETTUA TILAA
                if (now - powerStateStartTime >= 4000) {
                    Serial.println("PC power-off confirmed - turning relay OFF");
                    digitalWrite(OPTO_PIN, LOW);
                    digitalWrite(POWER_LED_PIN, LOW);
                    powerState = POWER_IDLE;
                    
                    Serial.println("PC OFF - Controller reset handled elsewhere");
                }
            } else {
                // PC ei ole vielä sammunut, nollataan ajastin
                powerStateStartTime = now;
            }
            break;
            
        case POWER_FORCE_START:
            Serial.println("Force shutdown - OPTO_PIN HIGH for 5000ms");
            digitalWrite(OPTO_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, HIGH);
            powerState = POWER_FORCE_WAITING;
            powerStateStartTime = now;
            break;
            
        case POWER_FORCE_WAITING:
            if (now - powerStateStartTime >= 5000) {
                Serial.println("Force shutdown pulse complete - waiting for PC to power off");
                digitalWrite(OPTO_PIN, LOW);
                forceShutdown = true;
                forceShutdownStartTime = now;
                powerState = POWER_OFF_WAITING_POWEROFF;
                powerStateStartTime = now;
            }
            break;
            
        default:
            break;
    }
}

void handlePcStates() {
    if (forceShutdown) {
        if (filteredPcState == LOW) {
            forceShutdown = false;
            digitalWrite(OPTO_PIN, LOW);
            digitalWrite(POWER_LED_PIN, LOW);
        }
    }
    
    // BLUETOOTH OHJAUS PC:N TILAN MUKAAN
    static bool lastBTPcState = false;
    if (filteredPcState != lastBTPcState) {
        if (filteredPcState == HIGH) {
            Serial.println("PC ON - DISABLE BLUETOOTH");
            btStop();
        } else {
            Serial.println("PC OFF - ENABLE BLUETOOTH");
            btStart();
        }
        lastBTPcState = filteredPcState;
    }
    
    updatePcState();
}

#endif // PC_CONTROL_H