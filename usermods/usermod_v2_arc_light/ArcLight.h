#pragma once

#include "KY40Encoder.h"
#include "StripUpdater.h"
#include "wled.h"

class ArcLight : public Usermod {
   private:
    unsigned long previousCheckTime = 0;
    unsigned long delay = 2;

    unsigned char select_state = 0;  // 0 = brightness 1 = color
    unsigned char button_state = HIGH;
    unsigned char prev_button_state = HIGH;
    CRGB fastled_col;
    CHSV prim_hsv;

    int8_t dtIo = 13;   // D6
    int8_t clkIo = 14;  // D7
    int8_t swIo = 4;    // D2

    byte currentColors[3];

    int brightness = 127;

    const unsigned char modeCount = 3;
    unsigned char buttonState = HIGH;
    unsigned char previouButtonState = HIGH;
    unsigned char mode = 0;

    static const char _name[];
    static const char _defaultBrightness[];
    static const char _colorTempLowerBound[];
    static const char _colorTempUpperBound[];
    static const char _colorTemperature[];
    static const char _colorTemperatureStep[];
    
    uint16_t colorTempLowerBound = 800;
    uint16_t colorTempUpperBound = 4000;
    uint16_t colorTemperature = 1800;
    uint16_t colorTemperatureStep = 100;

    StripUpdater* stripUpdater;
    KY40Encoder* encoder;

   public:
    void setup() {
        PinManagerPinType pins[3] = {
            {dtIo, false}, {clkIo, false}, {swIo, false}};

        if (!pinManager.allocateMultiplePins(pins, 3, PinOwner::UM_ARC_LIGHT)) {
            cleanup();
            return;
        }

        stripUpdater = new StripUpdater(strip.getLengthTotal(), colorTempLowerBound, colorTempUpperBound, colorTemperatureStep);
        stripUpdater->setBrightness(brightness);
        stripUpdater->setColorTemperature(colorTemperature);



        encoder = new KY40Encoder(dtIo, clkIo);
        encoder->onLeftTurn([&]() { handleLeftTurn(); });
        encoder->onRightTurn([&]() { handleRightTurn(); });
    }

    void handleLeftTurn() {
        switch (mode) {
            case 0:
                stripUpdater->decreaseBrightness(1);
                break;
            case 1:
                stripUpdater->decreaseArcLength(1);
                break;
            case 2:
                stripUpdater->decreasePosition();
                break;
            case 3:
                stripUpdater->decreaseColorTemperature();
                break;
        }
    }

    void handleRightTurn() {
        switch (mode) {
            case 0:
                stripUpdater->increaseBrightness(1);
                break;
            case 1:
                stripUpdater->increaseArcLength(1);
                break;
            case 2:
                stripUpdater->increasePosition();
                break;
            case 3:
                stripUpdater->increaseColorTemperature();
                break;
        }
    }

    void handleButtonState() {
        buttonState = digitalRead(swIo);
        if (previouButtonState != buttonState) {
            Serial.println("mode:" + mode);
            if (buttonState == LOW) {
                mode = (mode + 1) % 4;
                previouButtonState = buttonState;
                Serial.println("mode:" + mode);
            }
            previouButtonState = buttonState;
        }
    }

    void loop() {
        if (millis() - previousCheckTime >= delay) {
            encoder->process();
            handleButtonState();
            previousCheckTime = millis();
        }
    }

    void addToConfig(JsonObject& root) {
        JsonObject arcLight = root[FPSTR(_name)];
        if (arcLight.isNull()) {
            arcLight = root.createNestedObject(FPSTR(_name));
        }
        arcLight[FPSTR(_defaultBrightness)] = brightness;
        arcLight[FPSTR(_colorTempLowerBound)] = colorTempLowerBound;
        arcLight[FPSTR(_colorTempUpperBound)] = colorTempUpperBound;
        arcLight[FPSTR(_colorTemperature)] = colorTemperature;
        arcLight[FPSTR(_colorTemperatureStep)] = colorTemperatureStep;

        JsonArray pins = arcLight.createNestedArray("pins");
        pins.add(dtIo);
        pins.add(clkIo);
        pins.add(swIo);

        DEBUG_PRINTLN(F("ArchLight config saved."));
    }

    bool readFromConfig(JsonObject& root) {
        JsonObject top = root[FPSTR(_name)];
        if (top.isNull()) {
            DEBUG_PRINT(FPSTR(_name));
            DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
            return false;
        }

        brightness = top[FPSTR(_defaultBrightness)] | brightness;
        colorTempLowerBound = top[FPSTR(_colorTempLowerBound)] | colorTempLowerBound;
        colorTempUpperBound = top[FPSTR(_colorTempUpperBound)] | colorTempUpperBound;
        colorTemperature = top[FPSTR(_colorTemperature)] | colorTemperature;
        colorTemperatureStep = top[FPSTR(_colorTemperatureStep)] | colorTemperatureStep;
        

        JsonArray pins = top[FPSTR("pins")];
        if (!pins.isNull()) {
            dtIo = pins[0];
            clkIo = pins[1];
            swIo = pins[2];
        } 
        // else using defaults
        
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(" config (re)loaded."));

        // use "return !top["newestParameter"].isNull();" when updating Usermod
        // with new features
        return true;
    }

    void readFromJsonState(JsonObject& root) {
        if (root["arcLight"] != nullptr) {
            JsonObject arcLightRoot = root["arcLight"];
            this->stripUpdater->setBrightness(arcLightRoot["brightness"]);
            this->stripUpdater->setColorTemperature(arcLightRoot["colorTemperature"]);
        }
    }

    void cleanup() {
        // Only deallocate pins if we allocated them ;)
        if (dtIo != -1) {
            pinManager.deallocatePin(dtIo, PinOwner::UM_ARC_LIGHT);
            dtIo = -1;
        }
        if (clkIo != -1) {
            pinManager.deallocatePin(clkIo, PinOwner::UM_ARC_LIGHT);
            clkIo = -1;
        }
        if (swIo != -1) {
            pinManager.deallocatePin(swIo, PinOwner::UM_ARC_LIGHT);
            swIo = -1;
        }
    }

    ~ArcLight() {
        delete stripUpdater;
        delete encoder;
    }
};

const char ArcLight::_name[] PROGMEM = "ArcLight";
const char ArcLight::_defaultBrightness[] PROGMEM = "Startup brightness";
const char ArcLight::_colorTempLowerBound[] PROGMEM = "Minimal color temperature";
const char ArcLight::_colorTempUpperBound[] PROGMEM = "Maximal color temperature";
const char ArcLight::_colorTemperature[] PROGMEM = " Startup color temperature";
const char ArcLight::_colorTemperatureStep[] PROGMEM = "Color temperature change step";