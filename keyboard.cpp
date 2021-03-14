#include "keyboard.h"

KeyBoardDriver::KeyBoardDriver(InterruptManager* manager) 
    : InterruptHandler(0x01 + manager->HardwareInterruptOffset(), manager),
    dataport(0x60),
    commandport(0x64) {
    while (commandport.Read() & 0x1) {
        dataport.Read();
    }
    commandport.Write(0xae);
    commandport.Write(0x20);
    uint8_t status = (dataport.Read() | 1) & ~0x10;
    commandport.Write(0x60);
    dataport.Write(status);
    dataport.Write(0xf4);
}

KeyBoardDriver::~KeyBoardDriver() {}

void printf(const char*);

uint32_t KeyBoardDriver::HandleInterrupt(uint32_t esp) {
    uint8_t key = dataport.Read();
    char* foo = (char*)"UNHANDLED INTERRUPT 0X00";
    const char* hex = "0123456789ABCDEF";
    foo[22] = hex[(key >> 4) & 0x0f];
    foo[23] = hex[key & 0x0f];
    printf((const char*)foo);
    return esp;
}