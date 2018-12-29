#pragma once
#include "Pump.h"

class PumpList {
    int _listSize;
    int _speed;
    int _cupMilliliter;
    Pump **_list;

    public:
    PumpList(int listSize);

    void addPump(int index, Pump *pump);
    bool setupPumps();
    void runPump(int index, int percent);

    void setSpeed(int speed);
    void setCupMilliliter(int cupMilliliter);
};
