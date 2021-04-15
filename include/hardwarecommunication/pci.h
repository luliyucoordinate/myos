#ifndef __MYOS__HARDWARECOMMUNICATION__PCI_H
#define __MYOS__HARDWARECOMMUNICATION__PCI_H

#include "hardwarecommunication/port.h"
#include "common/types.h"
#include "hardwarecommunication/interrupts.h"
#include "drivers/driver.h"


namespace myos
{
    namespace hardwarecommunication
    {
        enum BaseAddressRegisterType {
            MemoryMapping = 0,
            InputOutput = 1
        };

        class BaseAddressRegister {
        public:
            bool prefetchable;
            myos::common::uint8_t* address;
            myos::common::uint32_t size;
            BaseAddressRegisterType type;
        };

        class PeripheralComponentInterconnectDeviceDescriptor {
        public:
            PeripheralComponentInterconnectDeviceDescriptor();
            ~PeripheralComponentInterconnectDeviceDescriptor();

            myos::common::uint32_t portBase;
            myos::common::uint32_t interrupt;

            myos::common::uint8_t bus;
            myos::common::uint8_t device;
            myos::common::uint8_t function;

            myos::common::uint16_t device_id;
            myos::common::uint16_t vendor_id;

            myos::common::uint8_t class_id;
            myos::common::uint8_t subclass_id;
            myos::common::uint8_t interface_id;
            myos::common::uint8_t revision;
        };

        class PeripheralComponentInterconnectController {
        public:
            PeripheralComponentInterconnectController();
            ~PeripheralComponentInterconnectController();

            myos::common::uint32_t Read(myos::common::uint8_t bus, 
                myos::common::uint8_t device, 
                myos::common::uint8_t function, 
                myos::common::uint8_t registeroffset);
            
            void write(myos::common::uint8_t bus, 
                myos::common::uint8_t device, 
                myos::common::uint8_t function, 
                myos::common::uint8_t registeroffset,
                myos::common::uint32_t value);

            bool DeviceHasFunctions(myos::common::uint8_t bus, myos::common::uint8_t device);

            void SelectDrivers(myos::drivers::DriverManager* driverManager, myos::hardwarecommunication::InterruptManager* interrupts);
            myos::drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, myos::hardwarecommunication::InterruptManager* interrupts);

            PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(myos::common::uint8_t bus, 
                myos::common::uint8_t device, 
                myos::common::uint8_t function);
            BaseAddressRegister GetBaseAddressRegister(myos::common::uint8_t bus, 
                myos::common::uint8_t device, 
                myos::common::uint8_t function,
                myos::common::uint8_t bar);
        private:
            Port32Bit dataPort;
            Port32Bit commandPort;
        };
    }
}

#endif