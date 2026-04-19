#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "LittleFS.h"
#include <Bluepad32.h>
#include <ArduinoJson.h>

#include "version.h"
#include "pins.h"
#include "ps5_simple.h"

// TÄRKEÄÄ: Include pc_control.h ENSIN, jotta PowerState tunnetaan
#include "pc_control.h"

// Globaalit muuttujat
WebServer server(80);

bool pcIsOn = false;
bool shutdownRequested = false;
bool forceShutdown = false;
unsigned long forceShutdownStartTime = 0;
const unsigned long forceShutdownDuration = 5000;

// WiFi-muuttujat
String wifiSSID = "";
String wifiPassword = "";
bool wifiConfigured = false;
bool apMode = false;

// PS5-muuttujat
String ps5MacAddress = "";
bool ps5Enabled = false;
bool ps5AutoConnect = false;
unsigned long lastPS5ConnectionAttempt = 0;

// OPTIMOIDUT INTERVALLIT
unsigned long lastPinRead = 0;
const unsigned long pinReadInterval = 50;

unsigned long lastServerHandle = 0;
const unsigned long serverHandleInterval = 20;

unsigned long lastPcStateHandle = 0;
const unsigned long pcStateHandleInterval = 50;

unsigned long lastButtonDebounce = 0;
const unsigned long debounceDelay = 50;

// Välimuistissa olevat pinnien tilat
bool cachedButtonState = HIGH;
bool lastStableButtonState = HIGH;
bool buttonPressed = false;

// Filtteröity PC:n tila
bool filteredPcState = false;
unsigned long lastPcChangeTime = 0;
const unsigned long pcStableDelay = 100;

// Power-tilakoneen muuttujat
PowerState powerState = POWER_IDLE;
unsigned long powerStateStartTime = 0;

// PS5-luokka
PS5Simple ps5Simple;

// ================ PROTOTYYPIT ================
bool getStablePcState();
void startPowerOn();
void startForceShutdown();
void startNormalShutdown();
void savePS5Config(bool enabled, String mac, bool autoConnect);

// ================ CALLBACK-FUNKTIOT ================
void onConnectedGamepad(GamepadPtr gp) {
    Serial.println("=== UUSI OHJAIN HAVAITTU ===");
    if (gp != nullptr) {
        ps5Simple.onControllerConnected(gp);
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    Serial.println("=== OHJAIN IRROTTUNUT ===");
    ps5Simple.onControllerDisconnected(gp);
}

#include "web_server.h"

// ================ WiFi-konfiguraatio ================

void saveWiFiConfig(String ssid, String pass) {
    File file = LittleFS.open("/wifi_config.json", "w");
    if (!file) return;
    
    StaticJsonDocument<200> doc;
    doc["ssid"] = ssid;
    doc["password"] = pass;
    
    serializeJson(doc, file);
    file.close();
    
    wifiSSID = ssid;
    wifiPassword = pass;
    wifiConfigured = true;
}

void loadWiFiConfig() {
    if (!LittleFS.begin(true)) {
        wifiConfigured = false;
        apMode = true;
        return;
    }
    
    if (!LittleFS.exists("/wifi_config.json")) {
        wifiConfigured = false;
        apMode = true;
        return;
    }
    
    File file = LittleFS.open("/wifi_config.json", "r");
    if (!file) return;
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        wifiConfigured = false;
        apMode = true;
        return;
    }
    
    wifiSSID = doc["ssid"] | "";
    wifiPassword = doc["password"] | "";
    
    wifiConfigured = (wifiSSID.length() > 0);
    apMode = !wifiConfigured;
}

bool connectToWiFi() {
    loadWiFiConfig();
    
    if (!wifiConfigured || wifiSSID.length() == 0) {
        apMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("BC-250-POWER-CONTROL", "");
        return true;
    }
    
    WiFi.mode(WIFI_STA);
    String hostname = "bc250-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.setHostname(hostname.c_str());
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(100);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        apMode = false;
        return true;
    } else {
        apMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("BC-250-POWER-CONTROL", "");
        return false;
    }
}

// ================ PC-tilan suodatus ================



// ================ PS5-funktiot ================

void savePS5Config(bool enabled, String mac, bool autoConnect) {
    File file = LittleFS.open("/ps5_config.json", "w");
    if (!file) return;
    
    StaticJsonDocument<300> doc;
    doc["enabled"] = enabled;
    
    if (mac.length() == 0 || mac == "00:00:00:00:00:00") {
        doc["macAddress"] = "";
    } else {
        doc["macAddress"] = mac;
    }
    
    doc["autoConnect"] = autoConnect;
    
    serializeJson(doc, file);
    file.close();
    
    ps5Enabled = enabled;
    ps5MacAddress = (mac.length() == 0 || mac == "00:00:00:00:00:00") ? "" : mac;
    ps5AutoConnect = autoConnect;
    
    ps5Simple.setAllowedMac(ps5MacAddress);
}

void loadPS5Config() {
    if (!LittleFS.exists("/ps5_config.json")) {
        ps5Enabled = false;
        ps5MacAddress = "";
        ps5AutoConnect = false;
        ps5Simple.setAllowedMac("");
        Serial.println("PS5: Ei konfiguraatiota - kaikki ohjaimet sallittu");
        return;
    }
    
    File file = LittleFS.open("/ps5_config.json", "r");
    if (!file) return;
    
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        ps5Enabled = false;
        ps5MacAddress = "";
        ps5AutoConnect = false;
        ps5Simple.setAllowedMac("");
        Serial.println("PS5: Konfiguraatio virheellinen - kaikki ohjaimet sallittu");
        return;
    }
    
    ps5Enabled = doc["enabled"] | false;
    ps5MacAddress = doc["macAddress"] | "";
    ps5AutoConnect = doc["autoConnect"] | false;
    
    ps5Simple.setAllowedMac(ps5MacAddress);
    
    Serial.print("PS5: Lataus valmis - MAC: '");
    Serial.print(ps5MacAddress);
    Serial.print("', enabled: ");
    Serial.println(ps5Enabled);
}

// ================ SETUP ================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== BC-250 STARTING ===");
    
    initPins();
    Serial.println("Pins initialized");
    
    Serial.print("PC_MONITOR_PIN (14): ");
    Serial.println(digitalRead(PC_MONITOR_PIN) ? "HIGH" : "LOW");
    Serial.print("OPTO_PIN (16): ");
    Serial.println(digitalRead(OPTO_PIN) ? "HIGH" : "LOW");
    Serial.print("EXTRA_PIN (32): ");  // LISÄTTY: EXTRA_PIN tila
    Serial.println(digitalRead(EXTRA_PIN) ? "HIGH" : "LOW");
    
    filteredPcState = digitalRead(PC_MONITOR_PIN);
    pcIsOn = filteredPcState;
    
    Serial.println("Loading WiFi config...");
    loadWiFiConfig();
    
    Serial.println("Connecting to WiFi...");
    connectToWiFi();
    
    Serial.print("WiFi mode: ");
    Serial.println(apMode ? "AP" : "STA");
    if (!apMode) {
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
    
    Serial.println("Mounting LittleFS...");
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed!");
    } else {
        Serial.println("LittleFS mounted");
    }

    Serial.println("Setting up Bluepad32...");
    
    // VAIN NÄMÄ KAKSI RIVIÄ TARVITAAN
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    BP32.enableVirtualDevice(false);
    
    Serial.println("Loading PS5 config...");
    loadPS5Config();
    
    Serial.println("Bluepad32 ready - waiting for controller pairing");
    
    Serial.println("Setting up web server...");
    setupWebServer();
    
    Serial.println("=== BC-250 READY ===\n");
}

void loop() {
    unsigned long now = millis();
    
    // LISÄTTY: DEBUG - tulosta powerState sekunnin välein
    static unsigned long lastStatePrint = 0;
    static PowerState lastPowerState = POWER_IDLE;
    
    if (powerState != lastPowerState) {
        // Tila on vaihtunut, tulosta uusi tila
        switch(powerState) {
            case POWER_IDLE: Serial.print("IDLE"); break;
            case POWER_ON_START: Serial.print("ON_START"); break;
            case POWER_ON_WAITING_RELAY2: Serial.print("ON_WAITING_RELAY2"); break;
            case POWER_ON_COMPLETE: Serial.print("ON_COMPLETE"); break;
            case POWER_OFF_START: Serial.print("OFF_START"); break;
            case POWER_OFF_WAITING: Serial.print("OFF_WAITING"); break;
            case POWER_OFF_WAITING_POWEROFF: Serial.print("OFF_WAITING_POWEROFF"); break;
            case POWER_FORCE_START: Serial.print("FORCE_START"); break;
            case POWER_FORCE_WAITING: Serial.print("FORCE_WAITING"); break;
            default: Serial.print("UNKNOWN"); break;
        }
        Serial.println();
        lastPowerState = powerState;
        lastStatePrint = now;
    }

    // Lue BUTTON_PIN
    if (now - lastPinRead >= pinReadInterval) {
        cachedButtonState = digitalRead(BUTTON_PIN);
        lastPinRead = now;
    }

    // Käsittele PC:n tilat
    if (now - lastPcStateHandle >= pcStateHandleInterval) {
        handlePcStates();
        lastPcStateHandle = now;
    }

    // Käsittele power-tilat
    handlePowerStates();

    // Web-palvelin
    if (now - lastServerHandle >= serverHandleInterval) {
        server.handleClient();
        lastServerHandle = now;
    }
    
    // PS5 päivitys
    ps5Simple.handle();
    
    // ================ KORJATTU PAINIKKEEN KÄSITTELY ================
    static unsigned long buttonPressStartTime = 0;
    static bool buttonPressDetected = false;
    
    // Tarkista painikkeen tila debouncella
    if (cachedButtonState != lastStableButtonState) {
        lastButtonDebounce = now;
        lastStableButtonState = cachedButtonState;
    }
    
    // Jos tila on vakaa (debounce ohi)
    if ((now - lastButtonDebounce) > debounceDelay) {
        
        // Painike painettiin alas (LOW)
        if (cachedButtonState == LOW && !buttonPressDetected) {
            buttonPressDetected = true;
            buttonPressStartTime = now;
            Serial.println("BUTTON: Painike painettu alas");
        }
        
        // Painike vapautettiin (HIGH)
        if (cachedButtonState == HIGH && buttonPressDetected) {
            unsigned long pressDuration = now - buttonPressStartTime;
            buttonPressDetected = false;
            
            Serial.print("BUTTON: Painike vapautettu - kesto: ");
            Serial.print(pressDuration);
            Serial.println(" ms");
            
            // Tarkista PC:n tila (vain IDLE-tilassa)
            if (powerState == POWER_IDLE) {
                bool pcOn = getStablePcState();
                
                Serial.print("DEBUG: getStablePcState() = ");
                Serial.print(pcOn ? "HIGH (PC ON)" : "LOW (PC OFF)");
                Serial.print(", digitalRead(PC_MONITOR_PIN) = ");
                Serial.println(digitalRead(PC_MONITOR_PIN) ? "HIGH" : "LOW");
                
                if (pcOn) {
                    // PC ON PÄÄLLÄ
                    if (pressDuration >= 5000) {
                        // Pitkä painallus (yli 5s) = PAKKOSAMMUTUS
                        Serial.println("BUTTON: Pitkä painallus (>5s) - PAKKOSAMMUTUS");
                        startForceShutdown();
                    } else {
                        // Lyhyt painallus (alle 5s) = NORMAALI SAMMUTUS
                        Serial.println("BUTTON: Lyhyt painallus (<5s) - NORMAALI SAMMUTUS");
                        startNormalShutdown();
                    }
                } else {
                    // PC ON SAMMUNUT
                    // Painalluksesta riippumatta = KÄYNNISTYS
                    Serial.println("BUTTON: PC sammunut - KÄYNNISTYS");
                    startPowerOn();
                }
            } else {
                Serial.print("BUTTON: Power-tila ei IDLE - komento hylätty. Nykyinen tila: ");
                switch(powerState) {
                    case POWER_IDLE: Serial.print("IDLE"); break;
                    case POWER_ON_START: Serial.print("ON_START"); break;
                    case POWER_ON_WAITING_RELAY2: Serial.print("ON_WAITING_RELAY2"); break;
                    case POWER_ON_COMPLETE: Serial.print("ON_COMPLETE"); break;
                    case POWER_OFF_START: Serial.print("OFF_START"); break;
                    case POWER_OFF_WAITING: Serial.print("OFF_WAITING"); break;
                    case POWER_OFF_WAITING_POWEROFF: Serial.print("OFF_WAITING_POWEROFF"); break;
                    case POWER_FORCE_START: Serial.print("FORCE_START"); break;
                    case POWER_FORCE_WAITING: Serial.print("FORCE_WAITING"); break;
                    default: Serial.print("UNKNOWN"); break;
                }
                Serial.println();
            }
        }
    }
    
    delay(1);
}