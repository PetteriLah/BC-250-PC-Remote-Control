#ifndef PS5_SIMPLE_H
#define PS5_SIMPLE_H

#include <Bluepad32.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"

// TÄRKEÄÄ: Include pc_control.h ENSIN, jotta PowerState tunnetaan
#include "pc_control.h"

// TÄRKEÄÄ: Nämä ovat ulkoisia muuttujia/funktioita, jotka määritellään muualla
extern bool ps5Enabled;
extern String ps5MacAddress;
extern bool getStablePcState();
extern void startPowerOn();
extern PowerState powerState;
extern void savePS5Config(bool enabled, String mac, bool autoConnect);

class PS5Simple {
private:
    GamepadPtr myController = nullptr;
    String allowedMac = "";           // Tallennettu MAC (tyhjä = kaikki sallittu)
    String lastSeenMac = "";          // Viimeksi nähty MAC
    unsigned long lastSeenTime = 0;   // Milloin viimeksi nähty
    bool macAutoSaved = false;        // Onko MAC jo automaattisesti tallennettu
    
public:
    // Asetetaan sallittu MAC (tyhjä = kaikki sallittu)
    void setAllowedMac(String mac) {
        mac.trim();
        mac.replace(":", "");
        mac.replace("-", "");
        mac.toUpperCase();
        
        if (mac.length() == 0 || mac == "000000000000") {
            allowedMac = "";
            Serial.println("PS5: Kaikki ohjaimet sallittu");
        } else {
            allowedMac = mac;
            Serial.println("PS5: Vain MAC " + allowedMac + " sallittu");
        }
        macAutoSaved = false;
    }
    
    // Palauta sallittu MAC muotoiltuna
    String getAllowedMac() {
        if (allowedMac.length() != 12) return "";
        
        String formatted = "";
        for (int i = 0; i < 12; i += 2) {
            if (i > 0) formatted += ":";
            formatted += allowedMac.substring(i, i+2);
        }
        return formatted;
    }
    
    // Yhdistetyn ohjaimen MAC
    String getConnectedMac() {
        return lastSeenMac;
    }
    
    // Onko ohjain yhdistetty? (Katsotaan onko viime näkemästä alle 5 sekuntia)
    bool isConnected() {
        if (lastSeenMac.length() == 0) return false;
        if (millis() - lastSeenTime < 5000) return true;
        return false;
    }
    
    // Katkaise yhteys
    void disconnect() {
        // Ei tehdä mitään - ohjain katkaisee itse yhteyden
    }
    
    // UUSI OHJAIN HAVAITTU
    void onControllerConnected(GamepadPtr gp) {
        if (gp == nullptr) return;
        
        // TÄRKEÄÄ: Tarkista onko PC päällä
        bool pcOn = getStablePcState();
        
        if (pcOn || powerState != POWER_IDLE) {
            Serial.println("PS5: PC ON - OHJAINTA EI HYVÄKSYTÄ");
            gp->disconnect();
            return;
        }
        
        // Haetaan MAC-osoite
        GamepadProperties prop = gp->getProperties();
        char macStr[18];
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                prop.btaddr[0], prop.btaddr[1], prop.btaddr[2],
                prop.btaddr[3], prop.btaddr[4], prop.btaddr[5]);
        
        String mac = String(macStr);
        Serial.print("PS5: Ohjain havaittu - MAC: ");
        Serial.println(mac);
        
        // Poistetaan erottimet vertailua varten
        String macClean = mac;
        macClean.replace(":", "");
        macClean.replace("-", "");
        macClean.toUpperCase();
        
        // TARKISTETAAN MAC
        bool sallittu = false;
        
        if (allowedMac.length() == 0) {
            sallittu = true;
        }
        else if (macClean == allowedMac) {
            sallittu = true;
        }
        
        if (sallittu) {
            // TALLENNETAAN MAC
            lastSeenMac = mac;
            lastSeenTime = millis();
            Serial.println("✅ PS5: Sallittu MAC havaittu!");
            
            // AUTOMAATTINEN TALLENNUS: Jos MAC-lukko ei ole päällä, tallenna tämä MAC
            if (allowedMac.length() == 0 && !macAutoSaved) {
                Serial.println("PS5: Ei MAC-lukkoa - tallennetaan tama MAC automaattisesti!");
                
                // Tallennetaan MAC konfiguraatioon
                ps5MacAddress = mac;
                savePS5Config(true, mac, false);
                
                // Päivitetään myös oma allowedMac
                setAllowedMac(mac);
                
                macAutoSaved = true;
                Serial.println("✅ PS5: MAC tallennettu automaattisesti!");
            }
            
            // HYLÄTÄÄN YHTEYS TAHALLA
            if (gp != nullptr) {
                gp->disconnect();
                Serial.println("PS5: Yhteys katkaistaan - kaytetaan vain MAC-tietoa");
            }
        } else {
            Serial.println("❌ PS5: Hylatty MAC - ei sallittu");
            if (gp != nullptr) {
                gp->disconnect();
            }
        }
    }
    
    // OHJAIN IRROTTUNUT
    void onControllerDisconnected(GamepadPtr gp) {
        Serial.println("PS5: Ohjain irrotettu");
    }
    
    // TARKISTETAAN ONKO SALLITTU OHJAIN LÄHETTYVILLÄ
    bool isAuthorizedControllerNearby() {
        return isConnected();
    }
    
    // "Lue PS-nappi" - palauttaa true jos sallittu ohjain lähellä
    bool psButtonPressed() {
        return isAuthorizedControllerNearby();
    }
    
    // PÄÄSÄHKIN
    void handle() {
        // Päivitä Bluepad32
        BP32.update();
        
        // Tarkista PC:n tila
        bool pcOn = getStablePcState();
        
        // JOS PC ON PÄÄLLÄ, NOLLATAAN KAIKKI OHJAINTIEDOT
        if (pcOn || powerState != POWER_IDLE) {
            if (lastSeenMac.length() > 0) {
                Serial.println("PS5: PC ON - nollataan ohjaintiedot");
                lastSeenMac = "";
                lastSeenTime = 0;
                macAutoSaved = false;
            }
            return;
        }
        
        // Jos PS5 ei käytössä, älä tee mitään
        if (!ps5Enabled) {
            return;
        }
        
        unsigned long now = millis();
        
        // Jos sallittu ohjain on lähellä
        if (isAuthorizedControllerNearby()) {
            static unsigned long lastTriggerTime = 0;
            
            // Jos viime näkemästä on alle 2 sekuntia ja edellisestä triggeristä yli 5s
            if (now - lastSeenTime < 2000 && now - lastTriggerTime > 5000) {
                Serial.println("PS5: Sallittu ohjain lahella - kaynnistetaan PC!");
                lastTriggerTime = now;
                startPowerOn();
            }
        }
        
        // Tulosta vain harvoin
        static unsigned long lastPrint = 0;
        if (now - lastPrint > 10000 && ps5Enabled) {
            if (isAuthorizedControllerNearby()) {
                Serial.println("PS5: Sallittu ohjain lahella...");
            } else {
                Serial.println("PS5: Odotetaan ohjainta...");
            }
            lastPrint = now;
        }
    }
};

extern PS5Simple ps5Simple;

#endif