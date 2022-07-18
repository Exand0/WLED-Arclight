//#include "KY40Encoder.h"
#include <Arduino.h>

class KY40Encoder {
    typedef std::function<void()> CallbackFunction;

   private:
    unsigned char pin0 = -1;
    unsigned char pin1 = -1;
    CallbackFunction callbackRight = NULL;
    CallbackFunction callbackLeft = NULL;
    uint16_t pin0SignalPrevious = 0;

   public:
    KY40Encoder(unsigned char pin0, unsigned char pin1)
        : pin0{pin0}, pin1{pin1} {};

    void onLeftTurn(CallbackFunction callback) {
        callbackLeft = callback;
    }

    void onRightTurn(CallbackFunction callback) {
        callbackRight = callback;
    }

    void process() {
        int pin0Signal = digitalRead(pin0);  // Read encoder pins
        int pin1Signal = digitalRead(pin1);
        if ((!pin0Signal) && (pin0SignalPrevious)) {    // A has gone from high to low
            if (pin1Signal == HIGH) {  // B is high so clockwise
                if (callbackRight != NULL) {
                    callbackRight();
                    Serial.println("right");
                }
            } else if (pin1Signal == LOW) {  // B is low so counter-clockwise
                if (callbackLeft != NULL) {
                    callbackLeft();
                    Serial.println("left");
                }
            }
        }
        pin0SignalPrevious = pin0Signal;  // Store value of A for next time
    }
};