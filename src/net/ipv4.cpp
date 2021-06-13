#include "net/ipv4.h"

using namespace myos;
using namespace myos::common;
using namespace myos::net;

InternetProtocolHandler::InternetProtocolHandler(InternetProtocolProvider* backend, uint8_t protocol) {
    this->backend = backend;
    this->protocol = protocol;
    backend->handlers[protocol] = this;
}

InternetProtocolHandler::~InternetProtocolHandler() {
    if (backend->handlers[protocol] == this) {
        backend->handlers[protocol] = 0;
    }
}

bool InternetProtocolHandler::OnInternetProtocolReceived(uint32_t srcIP_BE, 
    uint32_t dstIP_BE, 
    uint8_t* internetProtocolPayload, 
    uint32_t size) {
    return false;
}

void InternetProtocolHandler::Send(uint32_t dstIP_BE, uint8_t* internetProtocolPayload, uint32_t size) {
    backend->Send(dstIP_BE, protocol, internetProtocolPayload, size);
}

InternetProtocolProvider::InternetProtocolProvider(EtherFrameProvider* backend, 
    AddressResolutionProtocol* arp,
    uint32_t gatewayIP,
    uint32_t subnetMask)
    : EtherFrameHandler(backend, 0x800) {
    for (int i = 0; i < 255; i++) {
        handlers[i] = 0;
    }
    this->arp = arp;
    this->gatewayIP = gatewayIP;
    this->subnetMask = subnetMask;
}

bool InternetProtocolProvider::OnEtherFrameReceived(uint8_t* buffer, uint32_t size) {
    if (size < sizeof(InternetProtocolV4Message)) {
        return false;
    }

    InternetProtocolV4Message* ipMessage = (InternetProtocolV4Message*)buffer;
    bool sendBack = false;

    if (ipMessage->dstIP == backend->GetIPAddress()) {
        int length = ipMessage->totalLength;
        if (length > size) {
            length = size;
        }

        if (handlers[ipMessage->protocol] != 0) {
            sendBack = handlers[ipMessage->protocol]->OnInternetProtocolReceived(
                ipMessage->srcIP, ipMessage->dstIP, buffer + 4 * ipMessage->headerLength, length - 4 * ipMessage->headerLength
            );
        }
    }

    if (sendBack) {
        uint32_t tmp = ipMessage->dstIP;
        ipMessage->dstIP = ipMessage->srcIP;
        ipMessage->srcIP = tmp;

        ipMessage->timeToLive = 0x40;
        ipMessage->checkSum = 0;
        ipMessage->checkSum = CheckSum((uint16_t*)ipMessage, 4 * ipMessage->headerLength);
    }
    return sendBack;
}

void InternetProtocolProvider::Send(uint32_t dstIP_BE, uint8_t protocol, uint8_t* data, uint32_t size) {
    uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(InternetProtocolV4Message) + size);
    InternetProtocolV4Message* message = (InternetProtocolV4Message*)buffer;

    message->version = 4;
    message->headerLength = sizeof(InternetProtocolV4Message) / 4;
    message->tos = 0;
    message->totalLength = size + sizeof(InternetProtocolV4Message);
    message->totalLength = ((message->totalLength & 0xff00) >> 8) | ((message->totalLength & 0x00ff) << 8);

    message->ident = 0x0100;
    message->flagsAndOffset = 0x0040; // 0000 0000 0100 0000
    message->timeToLive = 0x40;
    message->protocol = protocol;

    message->dstIP = dstIP_BE;
    message->srcIP = backend->GetIPAddress();
    message->checkSum = 0;
    message->checkSum = CheckSum((uint16_t*)message, sizeof(InternetProtocolV4Message));

    uint8_t* databuffer = buffer + sizeof(InternetProtocolV4Message);

    for (int i = 0; i < size; i++) {
        databuffer[i] = data[i];
    }

    uint32_t route = dstIP_BE;
    if ((dstIP_BE & subnetMask) != (message->srcIP & subnetMask)) {
        route = gatewayIP;
    }

    backend->Send(arp->Resolve(route), this->etherType_BE, buffer, sizeof(InternetProtocolV4Message) + size);
    MemoryManager::activeMemoryManager->free(buffer);
}

uint16_t InternetProtocolProvider::CheckSum(uint16_t* data, uint32_t size) {
    uint32_t tmp = 0;
    for (int i = 0; i < size / 2; i++) {
        tmp += ((data[i] & 0xff00) >> 8) | ((data[i] & 0x00ff) << 8);
    }

    if (size % 2) tmp += ((uint16_t)((char*)data)[size - 1]) << 8;

    while (tmp & 0xffff0000) tmp = (tmp & 0xffff) + (tmp >> 16);

    return ((~tmp & 0xff00) >> 8) | ((~tmp & 0x00ff) << 8);
}