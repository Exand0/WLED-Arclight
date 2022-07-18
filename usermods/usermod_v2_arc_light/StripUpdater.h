#include "wled.h"

class StripUpdater {
   private:
    uint16_t length;
    uint16_t middle;
    uint16_t halfArcLength;
    uint16_t halfArcLengthMax;
    uint16_t halfArcLengthMin;

    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
    uint16_t white = 0;

    uint16_t colorTempLowerBound;
    uint16_t colorTempUpperBound;
    uint16_t colorTemperatureStep;
    uint16_t colorTemperature;

    CRGB fastledColor;
    CHSV hsv;

    static const int MAX_BRIGHTNESS = 250;
    static const int MIN_BRIGHTNESS = 10;

    void clear() {
        fill(0);
    }

    void fill(uint16_t c) {
        for (uint16_t i = 0; i < length; i++) {
            strip.setPixelColor(i, c);
        }
    }

   public:

    StripUpdater(uint16_t stripLength, uint16_t colorTempLowerBound, uint16_t colorTempUpperBound, uint16_t colorTempStep) :
        length{stripLength},
        colorTempLowerBound{colorTempLowerBound},
        colorTempUpperBound{colorTempUpperBound},
        colorTemperatureStep{colorTempStep} {
            middle = stripLength / 2;
            halfArcLength = getHalfArcLength(stripLength);
            halfArcLengthMax = halfArcLength;
            halfArcLengthMin = 0;
    };

    /*
     * Get half arc for a given led strip segment length
     */
    uint16_t static getHalfArcLength(uint16_t segmentLength) {
        uint16_t halfArcLength = 0;
        if (segmentLength % 2 != 0) {
            halfArcLength = segmentLength / 2;
        } else {
            halfArcLength = segmentLength / 2 + 1;
        }
        return halfArcLength;
    }

    void setArcHalfLength(uint16_t halfArcLength) {
        this->halfArcLength = halfArcLength;
        this->updateArc();
    }

    void increaseArcLength(uint16_t amount) {
        if (halfArcLength <= halfArcLengthMax) {
            this->halfArcLength += amount;
            this->updateArc();
        }
    }

    void decreaseArcLength(uint16_t amount) {
        if (halfArcLength > halfArcLengthMin) {
            this->halfArcLength -= amount;
            this->updateArc();
        }
    }

    void increasePosition() {
        if (middle < strip.getLengthTotal()) {
            middle++;
            this->updateArc();
        }
    }

    void decreasePosition() {
        if (middle > 0) {
            middle--;
            this->updateArc();
        }
    }
    
    void changeColorTemperature(bool negative, int step) {
        if (colorTemperature == -1 ) colorTemperature = approximateKelvinFromRGB(RGBW32(col[0],col[1],col[2],col[3]));

        if (negative) {
            if (colorTemperature > colorTempLowerBound) {
                colorTemperature -= colorTemperatureStep;
            }
        } else {
            if (colorTemperature < colorTempUpperBound) {
                colorTemperature += colorTemperatureStep;
            }
        }
        colorKtoRGB(colorTemperature, col);
        updateWledInternals();
    }

    void decreaseColorTemperature() {
        changeColorTemperature(true, colorTemperatureStep);
    }
    
    void increaseColorTemperature() {
        changeColorTemperature(false, colorTemperatureStep);
    }

    void setMiddle(uint16_t middle) {
        if (middle < 0) {
            this->middle = 0;
        } else if (middle > strip.getLengthTotal()) {
            this->middle = strip.getLengthTotal();
        } else {
            this->middle = middle;
        }
    }

    void setBrightness(int newBri) {
        if (newBri <= MAX_BRIGHTNESS && newBri >= MIN_BRIGHTNESS) {
            bri = newBri;
            updateWledInternals();
        }
    }

    void setColorTemperature(int newTemp) {
        if (newTemp < colorTempUpperBound && newTemp > colorTempLowerBound) {
            colorTemperature = newTemp;
        } else {
            if (newTemp > colorTempUpperBound) {
                colorTemperature = colorTempUpperBound;
            }
            if (newTemp < colorTempLowerBound) {
                colorTemperature = colorTempLowerBound;
            }
        }
       
        colorKtoRGB(colorTemperature, col);
        updateWledInternals();
    } 

    void increaseBrightness(int fade) {
        if (bri + fade <= MAX_BRIGHTNESS) {
            bri += fade;
            updateWledInternals();
        }
    }

    void decreaseBrightness(int fade) {
        if (bri - fade >= MIN_BRIGHTNESS) {
            bri -= fade;
            updateWledInternals();
        }
    }

    /*  void setRGBW(uint16_t change) {
         // color_wheel()
         fastledColor.red = col[0];
         fastledColor.green = col[1];
         fastledColor.blue = col[2];
         hsv = rgb2hsv_approximate(fastledColor);
         int16_t new_val = (int16_t)hsv.h + change;
         if (new_val > 255) new_val -= 255;  // roll-over if  bigger than 255
         if (new_val < 0) new_val += 255;    // roll-over if smaller than 0
         hsv.h = (byte)new_val;
         hsv2rgb_rainbow(hsv, fastledColor);
         col[0] = fastledColor.red;
         col[1] = fastledColor.green;
         col[2] = fastledColor.blue;
     } */

    /*
     * Update led strip segment based on current selected middle and arc length
     */
    void updateArc() {
        uint16_t newSegmentStart =
            (middle - halfArcLength < 0) ? 0 : middle - halfArcLength;
        uint16_t newSegmentEnd =
            ((middle + halfArcLength) >= strip.getLengthTotal())
                ? strip.getLengthTotal()
                : middle + halfArcLength;
        strip.setSegment(0, newSegmentStart, newSegmentEnd);
    }

    /*
     * Thease are called in order to actually update strip based on values
     */
    void updateWledInternals() {
        //Serial.println(bri);
        colorUpdated(CALL_MODE_BUTTON);
        updateInterfaces(CALL_MODE_BUTTON);
    }
};
