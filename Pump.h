#pragma once

class Pump {
    int _pin;

    public:
    Pump(int pin);
    bool setup();
    void run(int milliseconds);
};
