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

    static bool shift = false;
    switch (key) {
    case 0x02: if (shift) printf("!"); else printf("1"); break;
    case 0x03: if (shift) printf("@"); else printf("2"); break;
    case 0x04: if (shift) printf("#"); else printf("3"); break;
    case 0x05: if (shift) printf("$"); else printf("4"); break;
    case 0x06: if (shift) printf("%"); else printf("5"); break;
    case 0x07: if (shift) printf("^"); else printf("6"); break;
    case 0x08: if (shift) printf("&"); else printf("7"); break;
    case 0x09: if (shift) printf("*"); else printf("8"); break;
    case 0x0A: if (shift) printf("("); else printf("9"); break;
    case 0x0B: if (shift) printf(")"); else printf("0"); break;

    case 0x10: if (shift) printf("Q"); else printf("q"); break;
    case 0x11: if (shift) printf("W"); else printf("w"); break;
    case 0x12: if (shift) printf("E"); else printf("e"); break;
    case 0x13: if (shift) printf("R"); else printf("r"); break;
    case 0x14: if (shift) printf("T"); else printf("t"); break;
    case 0x15: if (shift) printf("Z"); else printf("z"); break;
    case 0x16: if (shift) printf("U"); else printf("u"); break;
    case 0x17: if (shift) printf("I"); else printf("i"); break;
    case 0x18: if (shift) printf("O"); else printf("o"); break;
    case 0x19: if (shift) printf("P"); else printf("p"); break;

    case 0x1E: if (shift) printf("A"); else printf("a"); break;
    case 0x1F: if (shift) printf("S"); else printf("s"); break;
    case 0x20: if (shift) printf("D"); else printf("d"); break;
    case 0x21: if (shift) printf("F"); else printf("f"); break;
    case 0x22: if (shift) printf("G"); else printf("g"); break;
    case 0x23: if (shift) printf("H"); else printf("h"); break;
    case 0x24: if (shift) printf("J"); else printf("j"); break;
    case 0x25: if (shift) printf("K"); else printf("k"); break;
    case 0x26: if (shift) printf("L"); else printf("l"); break;

    case 0x2C: if (shift) printf("Y"); else printf("y"); break;
    case 0x2D: if (shift) printf("X"); else printf("x"); break;
    case 0x2E: if (shift) printf("C"); else printf("c"); break;
    case 0x2F: if (shift) printf("V"); else printf("v"); break;
    case 0x30: if (shift) printf("B"); else printf("b"); break;
    case 0x31: if (shift) printf("N"); else printf("n"); break;
    case 0x32: if (shift) printf("M"); else printf("m"); break;
    case 0x33: if (shift) printf("<"); else printf(","); break;
    case 0x34: if (shift) printf(">"); else printf("."); break;
    case 0x35: if (shift) printf("_"); else printf("-"); break;

    case 0x1C: printf("\n"); break;
    case 0x39: printf(" "); break;
    case 0x2A: case 0x36: shift = true; break;
    case 0xAA: case 0xB6: shift = false; break;

    case 0x45: break; // NumLock

    default:
        if (key < 0x80) {
            char* foo = (char*)"KEYBOARD 0X00  ";
            const char* hex = "0123456789ABCDEF";
            foo[11] = hex[(key >> 4) & 0x0f];
            foo[12] = hex[key & 0x0f];
            printf((const char*)foo);
        }
    }
    
    return esp;
}