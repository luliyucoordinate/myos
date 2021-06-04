#include "drivers/amd_am79c973.h"
using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

amd_am79c973::amd_am79c973(PeripheralComponentInterconnectDeviceDescriptor* dev, InterruptManager* interrupts) 
    : Driver(),
      InterruptHandler(dev->interrupt + interrupts->HardwareInterruptOffset(), interrupts),
      MACAddress0Port(dev->portBase),
      MACAddress2Port(dev->portBase + 0x20),
      MACAddress4Port(dev->portBase + 0x40),
      registerDataPort(dev->portBase + 0x10),
      registerAddressPort(dev->portBase + 0x12),
      resetPort(dev->portBase + 0x14),
      busConstrolRegisterDataPort(dev->portBase + 0x16) {
    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    uint64_t MAC0 = MACAddress0Port.Read() % 256;
    uint64_t MAC1 = MACAddress0Port.Read() / 256;
    uint64_t MAC2 = MACAddress2Port.Read() % 256;
    uint64_t MAC3 = MACAddress2Port.Read() / 256;
    uint64_t MAC4 = MACAddress4Port.Read() % 256;
    uint64_t MAC5 = MACAddress4Port.Read() / 256;
    
    uint64_t MAC = MAC5 << 40 | MAC4 << 32 | MAC3 << 24 | MAC2 << 16 | MAC1 << 8 | MAC0;

    registerAddressPort.Write(20);
    busConstrolRegisterDataPort.Write(0x102);

    registerAddressPort.Write(0);
    registerDataPort.Write(0x04);

    initBlock.mode = 0;
    initBlock.reserved1 = 0;
    initBlock.numSendBuffers = 3;
    initBlock.reserved2 = 0;
    initBlock.numRecvBuffers = 3;
    initBlock.physicalAddress = MAC;
    initBlock.reserved3 = 0;
    initBlock.logicalAddress = 0;

    sendBufferDesc = (BufferDescriptor*)(((uint32_t)&sendBufferDescMemory[0] + 15) & 0xfff0);
    initBlock.sendBufferDescAddress = (uint32_t)sendBufferDesc;
    recvBufferDesc = (BufferDescriptor*)(((uint32_t)&recvBufferDescMemory[0] + 15) & 0xfff0);
    initBlock.recvBufferDescAddress = (uint32_t)recvBufferDesc;

    for (uint8_t i = 0; i < 8; i++) {
        sendBufferDesc[i].address = (((uint32_t)&sendBuffers[i] + 15) & 0xfff0);
        sendBufferDesc[i].flags = 0xf7ff;
        sendBufferDesc[i].flags2 = 0;
        sendBufferDesc[i].avail = 0;

        recvBufferDesc[i].address = (((uint32_t)&recvBuffers[i] + 15) & 0xfff0);
        recvBufferDesc[i].flags = 0xf7ff | 0x80000000;
        recvBufferDesc[i].flags2 = 0;
        recvBufferDesc[i].avail = 0;
    }

    registerAddressPort.Write(1);
    registerDataPort.Write((uint32_t)&initBlock);
    registerAddressPort.Write(2);
    registerDataPort.Write((uint32_t)&initBlock >> 16);
}

void amd_am79c973::Activate() {
    registerAddressPort.Write(0);
    registerDataPort.Write(0x41);

    registerAddressPort.Write(4);
    uint32_t temp = registerDataPort.Read();
    registerAddressPort.Write(4);
    registerDataPort.Write(temp | 0xC00);

    registerAddressPort.Write(0);
    registerDataPort.Write(0x42);
}

int amd_am79c973::Reset() {
    resetPort.Read();
    resetPort.Write(0);
    return 10;
}

void printf(const char*);

uint32_t amd_am79c973::HandleInterrupt(common::uint32_t esp) {
    printf("INTERRUPT FROM AMD am79c973\n");

    registerAddressPort.Write(0);
    uint32_t temp = registerDataPort.Read();

    if ((temp & 0x8000) == 0x8000) printf("AMD am79c973 ERROR\n");
    else if ((temp & 0x2000) == 0x2000) printf("AMD am79c973 COLLISION ERROR\n");
    else if ((temp & 0x1000) == 0x1000) printf("AMD am79c973 MISSED FRAME\n");
    else if ((temp & 0x0800) == 0x0800) printf("AMD am79c973 MEMORY ERROR\n");
    else if ((temp & 0x0400) == 0x0400) printf("AMD am79c973 DATA RECEVED\n");
    else if ((temp & 0x0200) == 0x0200) printf("AMD am79c973 DATA SEND\n");

    registerAddressPort.Write(0);
    registerDataPort.Write(temp);

    if ((temp & 0x0200) == 0x0200) printf("AMD am79c973 INIT DONE\n");
    return esp;
}