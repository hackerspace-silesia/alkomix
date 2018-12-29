#include "PumpList.h"

PumpList::PumpList(int listSize) : _listSize(listSize) {
    _list = new Pump*[_listSize];
}

void PumpList::addPump(int index, Pump *pump) {
    _list[index] = pump;
}

bool PumpList::setupPumps() {
    for (int i=0; i < _listSize; i++) {
        bool isOk = _list[i]->setup();
        if (!isOk) {
            return false;
        }
    }
    return true;
}

void PumpList::runPump(int index, int percent) {
    int milliseconds = _speed * _cupMilliliter * percent / 100;
    _list[index]->run(milliseconds);
}

void PumpList::setSpeed(int speed) {
    _speed = speed;
}

void PumpList::setCupMilliliter(int cupMilliliter) {
    _cupMilliliter = cupMilliliter;
}
