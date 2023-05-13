#include "SystemDataFiber.h"

#include <Esp.h>

bool SystemDataFiber::Data::operator==(const Data &o) const {
  return heapSize == o.heapSize && freeHeap == o.freeHeap ;
}
bool SystemDataFiber::Data::operator!=(const Data &o) const {
  return !(*this == o);
}

SystemDataFiber::SystemDataFiber() = default;
SystemDataFiber::~SystemDataFiber() = default;

void SystemDataFiber::setup() { }

void SystemDataFiber::loop() {
    Data data;

    data.heapSize = ESP.getHeapSize();
    data.freeHeap = ESP.getFreeHeap();

    _data = data;
    _freeHeap = data.freeHeap;
    _heapSize = data.heapSize;
}

const Observable<SystemDataFiber::Data> &SystemDataFiber::data() {
    return _data;
}

const Observable<uint32_t> &SystemDataFiber::heapSize() {
    return _heapSize;
}

const Observable<uint32_t> &SystemDataFiber::freeHeap() {
    return _freeHeap;
}
