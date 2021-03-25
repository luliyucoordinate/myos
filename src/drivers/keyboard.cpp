#include "drivers/keyboard.h"

using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

KeyboardEventHandler::KeyboardEventHandler() {}

void KeyboardEventHandler::OnKeyDown(char) {}

void KeyboardEventHandler::OnKeyUp(char) {}

KeyBoardDriver::KeyBoardDriver(InterruptManager* manager, KeyboardEventHandler* handler) 
    : InterruptHandler(0x01 + manager->HardwareInterruptOffset(), manager),
    dataport(0x60),
    commandport(0x64),
    handler(handler) {
    
}

KeyBoardDriver::~KeyBoardDriver() {}

void printf(const char*);
void printfHex(uint8_t);

void KeyBoardDriver::Activate() {
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

uint32_t KeyBoardDriver::HandleInterrupt(uint32_t esp) {
    uint8_t key = dataport.Read();
    if (handler == nullptr) return esp;

    static bool shift = false;
    switch (key) {
    case 0x02: if (shift) handler->OnKeyDown('!'); else handler->OnKeyDown('1'); break;
    case 0x03: if (shift) handler->OnKeyDown('@'); else handler->OnKeyDown('2'); break;
    case 0x04: if (shift) handler->OnKeyDown('#'); else handler->OnKeyDown('3'); break;
    case 0x05: if (shift) handler->OnKeyDown('$'); else handler->OnKeyDown('4'); break;
    case 0x06: if (shift) handler->OnKeyDown('%'); else handler->OnKeyDown('5'); break;
    case 0x07: if (shift) handler->OnKeyDown('^'); else handler->OnKeyDown('6'); break;
    case 0x08: if (shift) handler->OnKeyDown('&'); else handler->OnKeyDown('7'); break;
    case 0x09: if (shift) handler->OnKeyDown('*'); else handler->OnKeyDown('8'); break;
    case 0x0A: if (shift) handler->OnKeyDown('('); else handler->OnKeyDown('9'); break;
    case 0x0B: if (shift) handler->OnKeyDown(')'); else handler->OnKeyDown('0'); break;

    case 0x10: if (shift) handler->OnKeyDown('Q'); else handler->OnKeyDown('q'); break;
    case 0x11: if (shift) handler->OnKeyDown('W'); else handler->OnKeyDown('w'); break;
    case 0x12: if (shift) handler->OnKeyDown('E'); else handler->OnKeyDown('e'); break;
    case 0x13: if (shift) handler->OnKeyDown('R'); else handler->OnKeyDown('r'); break;
    case 0x14: if (shift) handler->OnKeyDown('T'); else handler->OnKeyDown('t'); break;
    case 0x15: if (shift) handler->OnKeyDown('Z'); else handler->OnKeyDown('z'); break;
    case 0x16: if (shift) handler->OnKeyDown('U'); else handler->OnKeyDown('u'); break;
    case 0x17: if (shift) handler->OnKeyDown('I'); else handler->OnKeyDown('i'); break;
    case 0x18: if (shift) handler->OnKeyDown('O'); else handler->OnKeyDown('o'); break;
    case 0x19: if (shift) handler->OnKeyDown('P'); else handler->OnKeyDown('p'); break;

    case 0x1E: if (shift) handler->OnKeyDown('A'); else handler->OnKeyDown('a'); break;
    case 0x1F: if (shift) handler->OnKeyDown('S'); else handler->OnKeyDown('s'); break;
    case 0x20: if (shift) handler->OnKeyDown('D'); else handler->OnKeyDown('d'); break;
    case 0x21: if (shift) handler->OnKeyDown('F'); else handler->OnKeyDown('f'); break;
    case 0x22: if (shift) handler->OnKeyDown('G'); else handler->OnKeyDown('g'); break;
    case 0x23: if (shift) handler->OnKeyDown('H'); else handler->OnKeyDown('h'); break;
    case 0x24: if (shift) handler->OnKeyDown('J'); else handler->OnKeyDown('j'); break;
    case 0x25: if (shift) handler->OnKeyDown('K'); else handler->OnKeyDown('k'); break;
    case 0x26: if (shift) handler->OnKeyDown('L'); else handler->OnKeyDown('l'); break;

    case 0x2C: if (shift) handler->OnKeyDown('Y'); else handler->OnKeyDown('y'); break;
    case 0x2D: if (shift) handler->OnKeyDown('X'); else handler->OnKeyDown('x'); break;
    case 0x2E: if (shift) handler->OnKeyDown('C'); else handler->OnKeyDown('c'); break;
    case 0x2F: if (shift) handler->OnKeyDown('V'); else handler->OnKeyDown('v'); break;
    case 0x30: if (shift) handler->OnKeyDown('B'); else handler->OnKeyDown('b'); break;
    case 0x31: if (shift) handler->OnKeyDown('N'); else handler->OnKeyDown('n'); break;
    case 0x32: if (shift) handler->OnKeyDown('M'); else handler->OnKeyDown('m'); break;
    case 0x33: if (shift) handler->OnKeyDown('<'); else handler->OnKeyDown(','); break;
    case 0x34: if (shift) handler->OnKeyDown('>'); else handler->OnKeyDown('.'); break;
    case 0x35: if (shift) handler->OnKeyDown('_'); else handler->OnKeyDown('-'); break;

    case 0x1C: handler->OnKeyDown('\n'); break;
    case 0x39: handler->OnKeyDown(' '); break;
    case 0x2A: case 0x36: shift = true; break;
    case 0xAA: case 0xB6: shift = false; break;

    case 0x45: break; // NumLock

    default:
        if (key < 0x80) {
            printf("KEYBOARD 0x");
            printfHex(key);
        }
    }
    
    return esp;
}