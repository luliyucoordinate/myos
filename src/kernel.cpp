#include "common/types.h"
#include "gdt.h"
#include "hardwarecommunication/interrupts.h"
#include "hardwarecommunication/pci.h"
#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

void printf(const char* str) {
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x = 0, y = 0;
    for (int i = 0; str[i]; i++) {
        switch(str[i]) {
        case '\n':
            y++;
            x = 0;
            break;
        default:
            VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | str[i];
            x++;
            break;
        }
        

        if (x >= 80) {
            x = 0;
            y++;
        }

        if (y >= 25) {
            for (y = 0; y < 25; y++) {
                for (x = 0; x < 80; x++) {
                    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | ' ';
                }
            }
            x = 0, y = 0;
        }
    }
}

void printfHex(uint8_t key) {
    char* foo = (char*)"00";
    const char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0x0f];
    foo[1] = hex[key & 0x0f];
    printf((const char*)foo);
}

class PrintKeyboardEventHandler : public KeyboardEventHandler {
public:
    void OnKeyDown(char c) {
        char* foo = (char*)" ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler {
public:
    MouseToConsole()
        : x(40),
          y(12) {
    }

    void OnActivate() {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[y * 80 + x] = ((VideoMemory[y * 80 + x] & 0xf000) >> 4) |
                                ((VideoMemory[y * 80 + x] & 0x0f00) << 4) |
                                (VideoMemory[y * 80 + x] & 0x00ff);
    }
    
    void OnMouseMove(int8_t nx, int8_t ny) {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[y * 80 + x] = ((VideoMemory[y * 80 + x] & 0xf000) >> 4) |
                                ((VideoMemory[y * 80 + x] & 0x0f00) << 4) |
                                (VideoMemory[y * 80 + x] & 0x00ff);

        x += nx;
        if (x < 0) x = 0;
        else if (x >= 80) x = 79;

        y += ny;
        if (y < 0) y = 0;
        else if (y >= 25) y = 24;

        VideoMemory[y * 80 + x] = ((VideoMemory[y * 80 + x] & 0xf000) >> 4) |
                                ((VideoMemory[y * 80 + x] & 0x0f00) << 4) |
                                (VideoMemory[y * 80 + x] & 0x00ff);
    }
    
private:
    int8_t x, y;
};

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(0x20, &gdt);

    DriverManager drvManager;
    PrintKeyboardEventHandler kbhandler;
    KeyBoardDriver keyboard(&interrupts, &kbhandler);
    drvManager.AddDriver(&keyboard);

    MouseToConsole mousehandler;
    MouseDriver mouse(&interrupts, &mousehandler);
    drvManager.AddDriver(&mouse);

    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager);
    drvManager.ActivateAll();

    interrupts.Activate();
    while(1);
}