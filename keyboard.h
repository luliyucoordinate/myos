#ifndef __KEY_BOARD_H
#define __KEY_BOARD_H

#include "types.h"
#include "interrupts.h"
#include "port.h"

class KeyBoardDriver : public InterruptHandler {
public:
    KeyBoardDriver(InterruptManager* manager);
    ~KeyBoardDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
private:
    Port8Bit dataport;
    Port8Bit commandport;
};

#endif 