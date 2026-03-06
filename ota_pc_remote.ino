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
#include "pc_control.h"  // Tämä tuo PowerState-enumin

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
int ps5State = 0;              // 0=disabled, 1=disconnected, 2=connecting, 3=connected
String ps5MacAddress = "";
bool ps5Enabled = false;
bool ps5AutoConnect = false;
unsigned long lastPS5ConnectionAttempt = 0;
unsigned long lastPS5Update = 0;

// INTERVALLIT PS5:LLE
const unsigned long ps5UpdateIntervalPcOn = 10000;      // 10s - PC päällä
const unsigned long ps5UpdateIntervalDisabled = 10000;  // 10s - PS5 ei käytössä
const unsigned long ps5UpdateIntervalDisconnected = 500; // 500ms - ei yhteyttä
const unsigned long ps5UpdateIntervalConnected = 500;   // 500ms - yhdistetty

// PS5-luokka
PS5Simple ps5Simple;

// OPTIMOIDUT INTERVALLIT
unsigned long lastPinRead = 0;
const unsigned long pinReadInterval = 50;     // 20 Hz (napin luku)

unsigned long lastServerHandle = 0;
const unsigned long serverHandleInterval = 20;  // 50 Hz (web-palvelin)

unsigned long lastPcStateHandle = 0;
const unsigned long pcStateHandleInterval = 50;  // 20 Hz (PC-tilan käsittely)

unsigned long lastButtonDebounce = 0;
const unsigned long debounceDelay = 50;

// Välimuistissa olevat pinnien tilat
bool cachedButtonState = HIGH;
bool lastStableButtonState = HIGH;
bool buttonPressed = false;

// Filtteröity PC:n tila
bool filteredPcState = false;
unsigned long lastPcChangeTime = 0;
const unsigned long pcStableDelay = 100; // 100ms vakautus

// Power-tilakoneen muuttujat - PowerState tunnetaan nyt
PowerState powerState = POWER_IDLE;
unsigned long powerStateStartTime = 0;

// TÄRKEÄÄ: MÄÄRITELLÄÄN CALLBACK-FUNKTIOT
void onConnectedGamepad(GamepadPtr gp) {
    Serial.println("Gamepad connected!");
    
    // Haetaan MAC-osoite GamepadProperties-rakenteesta
    GamepadProperties properties = gp->getProperties();
    Serial.printf("Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                  properties.btaddr[0], properties.btaddr[1], 
                  properties.btaddr[2], properties.btaddr[3],
                  properties.btaddr[4], properties.btaddr[5]);
    
    Serial.print("Gamepad model: ");
    Serial.println(gp->getModelName());
    
    ps5Simple.setController(gp);
}

void onDisconnectedGamepad(GamepadPtr gp) {
    Serial.println("Gamepad disconnected!");
    ps5Simple.clearController();
}

#include "web_server.h"

// ================ WiFi-konfiguraatio ================

// WiFi-konfiguraation tallennus ARDUINOJSONILLA
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

// WiFi-konfiguraation lataus ARDUINOJSONILLA
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

// WiFi-yhteys
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

// Yksinkertainen PC-tilan suodatus
bool getStablePcState() {
    static bool lastRawState = false;
    
    bool currentRaw = digitalRead(PC_MONITOR_PIN);
    unsigned long now = millis();
    
    if (currentRaw != lastRawState) {
        lastRawState = currentRaw;
        lastPcChangeTime = now;
        return filteredPcState;
    }
    
    if (now - lastPcChangeTime >= pcStableDelay) {
        filteredPcState = currentRaw;
    }
    
    return filteredPcState;
}

// ================ PS5-funktiot ================

// PS5 konfiguraation tallennus ARDUINOJSONILLA
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
    
    if (ps5MacAddress.length() == 0) {
        ps5Simple.setAllowedMac("00:00:00:00:00:00");
    } else {
        ps5Simple.setAllowedMac(ps5MacAddress);
    }
}

// PS5 konfiguraation lataus ARDUINOJSONILLA
void loadPS5Config() {
    if (!LittleFS.exists("/ps5_config.json")) {
        ps5Enabled = false;
        ps5MacAddress = "";
        ps5AutoConnect = false;
        ps5Simple.setAllowedMac("00:00:00:00:00:00");
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
        ps5Simple.setAllowedMac("00:00:00:00:00:00");
        return;
    }
    
    ps5Enabled = doc["enabled"] | false;
    ps5MacAddress = doc["macAddress"] | "";
    ps5AutoConnect = doc["autoConnect"] | false;
    
    if (ps5MacAddress.length() == 0) {
        ps5Simple.setAllowedMac("00:00:00:00:00:00");
    } else {
        ps5Simple.setAllowedMac(ps5MacAddress);
    }
}

void initPS5() {
    if (!ps5Enabled || ps5MacAddress.length() == 0 || getStablePcState()) {
        ps5State = 0;
        return;
    }
    
    if (ps5State == 0) {
        ps5State = 1;
        lastPS5ConnectionAttempt = millis();
    }
}

// PS5 handler - optimoitu versio
void handlePS5() {
    unsigned long now = millis();
    unsigned long interval;
    
    // 1. TARKISTETAAN PC:N TILA ENSIN
    bool pcOn = getStablePcState();
    
    // 2. VALITAAN INTERVALLI TILAN MUKAAN
    if (pcOn) {
        interval = ps5UpdateIntervalPcOn;
    } else if (!ps5Enabled || ps5MacAddress.length() == 0) {
        interval = ps5UpdateIntervalDisabled;
    } else if (ps5Simple.isConnected()) {
        interval = ps5UpdateIntervalConnected;
    } else {
        interval = ps5UpdateIntervalDisconnected;
    }
    
    // 3. TARKISTETAAN ONKO AIKA PÄIVITTÄÄ
    if (now - lastPS5Update < interval) return;
    lastPS5Update = now;
    
    // 4. PC PÄÄLLÄ - POISTETAAN KÄYTÖSTÄ
    if (pcOn) {
        if (ps5Simple.isConnected()) {
            ps5Simple.disconnect();
        }
        return;
    }
    
    // 5. PS5 EI KÄYTÖSSÄ
    if (!ps5Enabled || ps5MacAddress.length() == 0) {
        if (ps5Simple.isConnected()) {
            ps5Simple.disconnect();
        }
        return;
    }
    
    // 6. PÄIVITETÄÄN BLUEPAD32
    ps5Simple.update();
    
    // 7. YRITETÄÄN YHDISTÄÄ JOS EI YHTEYTTÄ
    if (!ps5Simple.isConnected()) {
        ps5Simple.begin(ps5MacAddress);
        return;
    }
    
    // 8. LUETAAN PS-NAPPI
    static bool lastButtonState = false;
    bool currentButton = ps5Simple.readPSButton();
    
    if (currentButton && !lastButtonState) {
        static unsigned long lastPress = 0;
        if (now - lastPress > 2000) {
            lastPress = now;
            ps5Simple.rumbleShort();
            startPowerOn();
        }
    }
    lastButtonState = currentButton;
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
    
    // Alustetaan PC:n tila
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

    Serial.println("Loading PS5 config...");
    loadPS5Config();
    
    Serial.println("Setting up Bluepad32...");
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    BP32.enableVirtualDevice(false);
    
    initPS5();
    
    Serial.println("Setting up web server...");
    setupWebServer();
    
    Serial.println("=== BC-250 READY ===\n");
}

// ================ LOOP ================

void loop() {
    unsigned long now = millis();
    
    // Lue BUTTON_PIN - 20 Hz
    if (now - lastPinRead >= pinReadInterval) {
        cachedButtonState = digitalRead(BUTTON_PIN);
        lastPinRead = now;
    }
    
    // Käsittele PC:n tilat - 20 Hz
    if (now - lastPcStateHandle >= pcStateHandleInterval) {
        handlePcStates();
        lastPcStateHandle = now;
    }
    
    // Käsittele power-tilat - AINA
    handlePowerStates();
    
    // Web-palvelin - 50 Hz
    if (now - lastServerHandle >= serverHandleInterval) {
        server.handleClient();
        lastServerHandle = now;
    }
    
    // PS5 päivitys
    handlePS5();
    
    // Painikkeen käsittely debouncella
    if (cachedButtonState != lastStableButtonState) {
        lastButtonDebounce = now;
        lastStableButtonState = cachedButtonState;
    }
    
    if ((now - lastButtonDebounce) > debounceDelay) {
        if (cachedButtonState == LOW && !buttonPressed) {
            buttonPressed = true;
            if (getStablePcState()) {
                startForceShutdown();
            } else {
                startPowerOn();
            }
        }
        
        if (cachedButtonState == HIGH && buttonPressed) {
            buttonPressed = false;
        }
    }
    
    delay(1);
}
