#ifndef PS5_SIMPLE_H
#define PS5_SIMPLE_H

#include <Bluepad32.h>
#include <Arduino.h>

// Vakio "kaikki sallittu" MAC-osoitteelle
#define MAC_ALL_ALLOWED "00:00:00:00:00:00"

class PS5Simple {
private:
    GamepadPtr myController = nullptr;
    bool connected = false;
    String allowedMacAddress = "";
    String currentControllerMac = "";
    
public:
    void setAllowedMac(String mac) {
        // Alkuperäinen MAC sellaisenaan
        allowedMacAddress = mac;
        allowedMacAddress.trim();
        
        // Jos MAC on tyhjä, muuta se ALL_ALLOWED muotoon
        if (allowedMacAddress.length() == 0) {
            allowedMacAddress = MAC_ALL_ALLOWED;
            Serial.println("🔓 MAC lock disabled (empty) - all controllers allowed");
            return;
        }
        
        // Tarkista onko kyseessä ALL_ALLOWED
        String tempMac = allowedMacAddress;
        tempMac.replace(":", "");
        tempMac.replace("-", "");
        tempMac.toUpperCase();
        
        if (tempMac == "000000000000") {
            Serial.println("🔓 MAC lock disabled (00:00:00:00:00:00) - all controllers allowed");
            return;
        }
        
        // Muuten kyseessä on oikea MAC-lukitus
        // Normalisoidaan MAC-osoite (poistetaan erikoismerkit)
        allowedMacAddress.replace(":", "");
        allowedMacAddress.replace("-", "");
        allowedMacAddress.toUpperCase();
        
        Serial.println("🔒 MAC lock enabled: " + allowedMacAddress);
    }
    
    String getAllowedMac() {
        // Palautetaan alkuperäinen muoto (kutsujan kannalta)
        if (allowedMacAddress == MAC_ALL_ALLOWED || allowedMacAddress.length() == 0) {
            return "";
        }
        
        // Muotoillaan MAC takaisin xx:xx:xx:xx:xx:xx muotoon näyttöä varten
        String formatted = allowedMacAddress;
        if (formatted.length() == 12) {
            String result = "";
            for (int i = 0; i < 12; i += 2) {
                if (i > 0) result += ":";
                result += formatted.substring(i, i+2);
            }
            return result;
        }
        return allowedMacAddress;
    }
    
    String getControllerMac() {
        return currentControllerMac;
    }
    
    bool begin(String mac) {
        Serial.println("PS5: Initializing...");
        setAllowedMac(mac);
        connected = false;
        return true;
    }
    
    void disconnect() {
        if (myController != nullptr) {
            myController->disconnect();
            myController = nullptr;
        }
        connected = false;
        currentControllerMac = "";
    }
    
    bool isConnected() {
        return (myController != nullptr && myController->isConnected());
    }
    
    void setController(GamepadPtr gp) {
        // HAETAAN MAC-OSOITE GamepadProperties-rakenteesta
        GamepadProperties properties = gp->getProperties();
        
        // Muodosta MAC-osoite stringiksi properties.btaddr taulukosta
        char macStr[18];
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                properties.btaddr[0], properties.btaddr[1], 
                properties.btaddr[2], properties.btaddr[3],
                properties.btaddr[4], properties.btaddr[5]);
        currentControllerMac = String(macStr);
        
        Serial.print("Controller identifier: ");
        Serial.println(currentControllerMac);
        Serial.print("Gamepad model: ");
        Serial.println(gp->getModelName());
        
        // Tarkista onko MAC-lukitus käytössä
        bool macLockEnabled = (allowedMacAddress != MAC_ALL_ALLOWED && 
                               allowedMacAddress.length() > 0 && 
                               allowedMacAddress != "000000000000");
        
        if (macLockEnabled) {
            // MAC LUKITUS ON KÄYTÖSSÄ
            String macForCompare = currentControllerMac;
            macForCompare.replace(":", "");
            macForCompare.replace("-", "");
            macForCompare.toUpperCase();
            
            Serial.print("Comparing with allowed: ");
            Serial.println(allowedMacAddress);
            
            if (macForCompare != allowedMacAddress) {
                Serial.println("⛔ UNAUTHORIZED CONTROLLER REJECTED!");
                gp->disconnect();
                return;
            } else {
                Serial.println("✅ Authorized controller accepted");
            }
        } else {
            // EI MAC LUKITUSTA - SALLITAAN KAIKKI
            Serial.println("🔓 No MAC restriction - allowing all controllers");
        }
        
        // HYVÄKSYTÄÄN OHJAIN
        myController = gp;
        connected = true;
        Serial.println("✅ PS5 controller connected!");
        
        // Kevyt värinä
        playRumble(50, 100);
    }
    
    void clearController() {
        myController = nullptr;
        connected = false;
        currentControllerMac = "";
        Serial.println("PS5 controller disconnected!");
    }
    
    bool readPSButton() {
        if (!isConnected()) return false;
        
        uint8_t misc = myController->miscButtons();
        
        if (misc & 0x01) {
            Serial.println("PS BUTTON PRESSED!");
            return true;
        }
        
        return false;
    }
    
    void playRumble(uint16_t delayedStartMs, uint8_t durationMs) {
        if (!isConnected()) return;
        
        myController->playDualRumble(
            delayedStartMs,
            durationMs,
            0xFF,
            0xFF
        );
    }
    
    void rumbleShort() {
        playRumble(20, 50);
    }
    
    void rumbleLong() {
        playRumble(50, 400);
    }
    
    void update() {
        BP32.update();
    }
};

extern PS5Simple ps5Simple;

#endif