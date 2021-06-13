#include "drivers/amd_am79c973.h"
using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

RawDataHandler::RawDataHandler(amd_am79c973* backend) {
    this->backend = backend;
    backend->SetHandler(this);
}

RawDataHandler::~RawDataHandler() {
    backend->SetHandler(0);
}

bool RawDataHandler::OnRawDataReceived(uint8_t* buffer, uint32_t size) {
    return false;
}

void RawDataHandler::Send(uint8_t* buffer, uint32_t size) {
    backend->Send(buffer, size);
}

amd_am79c973::amd_am79c973(PeripheralComponentInterconnectDeviceDescriptor* dev, InterruptManager* interrupts) 
    : Driver(),
      InterruptHandler(dev->interrupt + interrupts->HardwareInterruptOffset(), interrupts),
      MACAddress0Port(dev->portBase),
      MACAddress2Port(dev->portBase + 0x02),
      MACAddress4Port(dev->portBase + 0x04),
      registerDataPort(dev->portBase + 0x10),
      registerAddressPort(dev->portBase + 0x12),
      resetPort(dev->portBase + 0x14),
      busConstrolRegisterDataPort(dev->portBase + 0x16) {

    handler = 0;
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
void printfHex(uint8_t);

uint32_t amd_am79c973::HandleInterrupt(common::uint32_t esp) {
    registerAddressPort.Write(0);
    uint32_t temp = registerDataPort.Read();

    if ((temp & 0x8000) == 0x8000) printf("AMD am79c973 ERROR\n");
    else if ((temp & 0x2000) == 0x2000) printf("AMD am79c973 COLLISION ERROR\n");
    else if ((temp & 0x1000) == 0x1000) printf("AMD am79c973 MISSED FRAME\n");
    else if ((temp & 0x0800) == 0x0800) printf("AMD am79c973 MEMORY ERROR\n");
    else if ((temp & 0x0400) == 0x0400) Receive();
    else if ((temp & 0x0200) == 0x0200) printf(" SEND\n");

    registerAddressPort.Write(0);
    registerDataPort.Write(temp);

    if ((temp & 0x0100) == 0x0100) printf("AMD am79c973 INIT DONE\n");
    return esp;
}

void amd_am79c973::Send(uint8_t* buffer, int size) {
    int sendDesc = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1) % 8;
    if (size > 1518) size = 1518;

    for (uint8_t* src = buffer + size -1,
        *dst = (uint8_t*)(sendBufferDesc[sendDesc].address + size - 1);
        src >= buffer; src--, dst--) *dst = *src;
    
    printf("\nSENDING: ");
    for (int i = 0; i < (size > 64 ? 64 : size); i++) {
        printfHex(buffer[i]);
        printf(" ");
    }
    sendBufferDesc[sendDesc].avail = 0;
    sendBufferDesc[sendDesc].flags = 0x8300f000 | ((uint16_t)((-size) & 0xfff));
    sendBufferDesc[sendDesc].flags2 = 0;
    registerAddressPort.Write(0);
    registerDataPort.Write(0x48);
}

void amd_am79c973::Receive() {
    printf("\nRECEIVING: ");

    for (;(recvBufferDesc[currentRecvBuffer].flags & 0x80000000) == 0;
        currentRecvBuffer = (currentRecvBuffer + 1) % 8) {
        if (!(recvBufferDesc[currentRecvBuffer].flags & 0x40000000) && 
            (recvBufferDesc[currentRecvBuffer].flags & 0x03000000) == 0x03000000) { 
            uint32_t size = recvBufferDesc[currentRecvBuffer].flags & 0xfff;
            if (size > 64) size -= 4;

            uint8_t* buffer = (uint8_t*)(recvBufferDesc[currentRecvBuffer].address);
            for (int i = 0; i < (size > 64 ? 64 : size); i++) {
                printfHex(buffer[i]);
                printf(" ");
            }

            if (handler != 0) {
                if (handler->OnRawDataReceived(buffer, size)) {
                    Send(buffer, size);
                }
            }
        }

        recvBufferDesc[currentRecvBuffer].flags2 = 0;
        recvBufferDesc[currentRecvBuffer].flags = 0x8000f7ff;
    }
}

void amd_am79c973::SetHandler(RawDataHandler* handler) {
    this->handler = handler;
}

uint64_t amd_am79c973::GetMACAddress() {
    return initBlock.physicalAddress;
}

void amd_am79c973::SetIPAddress(uint32_t ip_be) {
    initBlock.logicalAddress = ip_be;
}

uint32_t amd_am79c973::GetIPAddress() {
    return initBlock.logicalAddress;
}