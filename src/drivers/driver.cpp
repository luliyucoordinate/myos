#include "drivers/driver.h"
#include "common/types.h"

using namespace myos::common;
using namespace myos::drivers;


Driver::Driver() {}
Driver::~Driver() {}

void Driver::Activate() {}

int Driver::Reset() {
    return 0;
}

void Driver::Deactivate() {}

DriverManager::DriverManager()
    : numDrivers(0) {
}

void DriverManager::AddDriver(Driver *drv) {
    drivers[numDrivers++] = drv;
}

void DriverManager::ActivateAll() {
    for (uint16_t i = 0; i < numDrivers; i++) {
        drivers[i]->Activate();
    }
}