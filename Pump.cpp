#include <Arduino.h>
#include "Pump.h"

Pump::Pump(int pin) : _pin(pin) {};

bool Pump::setup() {
    pinMode(_pin, OUTPUT); 
    digitalWrite(_pin, LOW);
    return true;
}

void Pump::run(int milliseconds) {
    digitalWrite(_pin, HIGH);
    delay(milliseconds);
    digitalWrite(_pin, LOW);
}
