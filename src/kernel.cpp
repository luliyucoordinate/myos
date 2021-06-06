#include "common/types.h"
#include "gdt.h"
#include "hardwarecommunication/interrupts.h"
#include "hardwarecommunication/pci.h"
#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "gui/desktop.h"
#include "gui/window.h"
#include "multitasking.h"
#include "memorymanagement.h"
#include "drivers/amd_am79c973.h"

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

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
    
    void OnMouseMove(int8_t nx, int8_t ny) override {
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

void taskA() {
    while (true) {
        printf("A");
    }
}

void taskB() {
    while (true) {
        printf("B");
    }
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {
    GlobalDescriptorTable gdt;

    size_t heap = 10 * 1024 * 1024;
    uint32_t* memupper = (uint32_t*)((size_t)multiboot_structure + 8);
    printfHex(((*memupper) >> 24) & 0xff);
    printfHex(((*memupper) >> 16) & 0xff);
    printfHex(((*memupper) >> 8) & 0xff);
    printfHex(((*memupper) >> 0) & 0xff);

    MemoryManager memoryManager(heap, (*memupper) * 1024 - heap - 10 * 1024);

    printf("\n heap: 0x");
    printfHex((heap >> 24) & 0xff);
    printfHex((heap >> 16) & 0xff);
    printfHex((heap >> 8) & 0xff);
    printfHex((heap >> 0) & 0xff);

    void* allocated = memoryManager.malloc(1024);
    printf("\n allocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xff);
    printfHex(((size_t)allocated >> 16) & 0xff);
    printfHex(((size_t)allocated >> 8) & 0xff);
    printfHex(((size_t)allocated >> 0) & 0xff);

    TaskManger taskManger;
    // Task task1(&gdt, taskA);
    // Task task2(&gdt, taskB);
    // taskManger.AddTask(&task1);
    // taskManger.AddTask(&task2);

    InterruptManager interrupts(0x20, &gdt, &taskManger);

// #define GRAPHICMODE
#ifdef GRAPHICMODE 
    Desktop desktop(320, 200, 0x00, 0x00, 0xa8);
#endif
    DriverManager drvManager;
    
#ifdef GRAPHICMODE 
    KeyBoardDriver keyboard(&interrupts, &desktop);
#else
    PrintKeyboardEventHandler kbhandler;
    KeyBoardDriver keyboard(&interrupts, &kbhandler);
#endif
    drvManager.AddDriver(&keyboard);

#ifdef GRAPHICMODE 
    MouseDriver mouse(&interrupts, &desktop);
#else
    MouseToConsole mousehandler;
    MouseDriver mouse(&interrupts, &mousehandler);
#endif
    drvManager.AddDriver(&mouse);

    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
    VideoGraphicsArray vga;

    drvManager.ActivateAll();

#ifdef GRAPHICMODE
    vga.SetMode(320, 200, 8);
    Window w1(&desktop, 10, 10, 20, 20, 0xa8, 0x00, 0x00);
    desktop.AddChild(&w1);
    Window w2(&desktop, 40, 15, 30, 30, 0x00, 0xa8, 0x00);
    desktop.AddChild(&w2);
#endif

    amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);
    eth0->Send((uint8_t*)"Hello Network", 13);

    interrupts.Activate();
    
    while(1) {
#ifdef GRAPHICMODE
        desktop.Draw(&vga);
#endif
    }
}