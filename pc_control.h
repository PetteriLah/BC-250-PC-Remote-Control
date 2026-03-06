#ifndef PC_CONTROL_H
#define PC_CONTROL_H

#include "pins.h"

// Power-tilakoneen tilat
enum PowerState {
    POWER_IDLE,
    POWER_ON_START,
    POWER_ON_WAITING,
    POWER_ON_COMPLETE,
    POWER_OFF_START,
    POWER_OFF_WAITING,
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

void initPins() {
    pinMode(OPTO_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(POWER_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(PC_MONITOR_PIN, INPUT);
    
    // Alkutila
    bool initial = digitalRead(PC_MONITOR_PIN);
    digitalWrite(OPTO_PIN, initial ? HIGH : LOW);
    digitalWrite(POWER_LED_PIN, initial ? HIGH : LOW);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    filteredPcState = initial;
    pcIsOn = initial;
    powerState = POWER_IDLE;
}

// PC:N TILAN PÄIVITYS
void updatePcState() {
    if (filteredPcState != pcIsOn) {
        if (filteredPcState == HIGH) {
            pcIsOn = true;
            shutdownRequested = false;
            forceShutdown = false;
            
            digitalWrite(OPTO_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, HIGH);
        } else {
            pcIsOn = false;
            shutdownRequested = false;
            forceShutdown = false;
            
            digitalWrite(OPTO_PIN, LOW);
            digitalWrite(POWER_LED_PIN, LOW);
        }
    }
}

// KÄYNNISTYS - TILAKONEVERSIO
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

// NORMAALI SAMMUTUS - TILAKONEVERSIO
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

// PAKKOSAMMUTUS - TILAKONEVERSIO
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
            Serial.println("Setting OPTO_PIN HIGH (pin 16)");
            digitalWrite(OPTO_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, HIGH);
            powerState = POWER_ON_WAITING;
            powerStateStartTime = now;
            break;
            
        case POWER_ON_WAITING:
            if (now - powerStateStartTime >= 3000) {
                Serial.println("Setting OPTO_PIN LOW");
                digitalWrite(OPTO_PIN, LOW);
                powerState = POWER_ON_COMPLETE;
                powerStateStartTime = now;
            }
            break;
            
        case POWER_ON_COMPLETE:
            if (now - powerStateStartTime >= 10) {
                Serial.print("OPTO_PIN final state: ");
                Serial.println(digitalRead(OPTO_PIN) ? "HIGH" : "LOW");
                Serial.println("Power on sequence complete");
                powerState = POWER_IDLE;
            }
            break;
            
        case POWER_OFF_START:
            Serial.println("Starting normal shutdown - OPTO_PIN HIGH for 500ms");
            digitalWrite(OPTO_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, LOW);
            powerState = POWER_OFF_WAITING;
            powerStateStartTime = now;
            break;
            
        case POWER_OFF_WAITING:
            if (now - powerStateStartTime >= 500) {
                Serial.println("Normal shutdown pulse complete");
                digitalWrite(OPTO_PIN, LOW);
                shutdownRequested = true;
                powerState = POWER_IDLE;
            }
            break;
            
        case POWER_FORCE_START:
            Serial.println("Starting force shutdown - OPTO_PIN HIGH for 5000ms");
            digitalWrite(OPTO_PIN, HIGH);
            digitalWrite(POWER_LED_PIN, HIGH);
            powerState = POWER_FORCE_WAITING;
            powerStateStartTime = now;
            break;
            
        case POWER_FORCE_WAITING:
            if (now - powerStateStartTime >= 5000) {
                Serial.println("Force shutdown pulse complete");
                digitalWrite(OPTO_PIN, LOW);
                forceShutdown = true;
                forceShutdownStartTime = now;
                powerState = POWER_IDLE;
            }
            break;
            
        default:
            break;
    }
}

// TILOJEN HALLINTA
void handlePcStates() {
    if (forceShutdown) {
        if (filteredPcState == LOW) {
            forceShutdown = false;
            digitalWrite(OPTO_PIN, LOW);
            digitalWrite(POWER_LED_PIN, LOW);
        }
    }
    
    updatePcState();
}

#endif // PC_CONTROL_H