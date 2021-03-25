#ifndef __MYOS__HARDWARECOMMUNICATION__PORT_H
#define __MYOS__HARDWARECOMMUNICATION__PORT_H

#include "common/types.h"

namespace myos
{
    namespace hardwarecommunication
    {
        class Port {
        protected:
            myos::common::uint16_t portnumber;
            Port(myos::common::uint16_t portnumber);
            ~Port();
        };

        class Port8Bit : public Port {
        public:
            Port8Bit(myos::common::uint16_t portnumber);
            ~Port8Bit();
            virtual void Write(myos::common::uint8_t data);
            virtual myos::common::uint8_t Read();
        protected:
            static inline myos::common::uint8_t Read8(myos::common::uint16_t _port) {
                myos::common::uint8_t result;
                __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (_port));
                return result;
            }
            static inline void Write8(myos::common::uint16_t _port, myos::common::uint8_t _data) {
                __asm__ volatile("outb %0, %1" : : "a" (_data), "Nd" (_port));
            }
        };

        class Port8BitSlow : public Port8Bit {
        public:
            Port8BitSlow(myos::common::uint16_t portnumber);
            ~Port8BitSlow();
            virtual void Write(myos::common::uint8_t data);
        protected:
            static inline void Write8Slow(myos::common::uint16_t _port, myos::common::uint8_t _data) {
                __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
            }
        };

        class Port16Bit : public Port {
        public:
            Port16Bit(myos::common::uint16_t portnumber);
            ~Port16Bit();
            virtual void Write(myos::common::uint16_t data);
            virtual myos::common::uint16_t Read();
        protected:
            static inline myos::common::uint16_t Read16(myos::common::uint16_t _port) {
                myos::common::uint16_t result;
                __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (_port));
                return result;
            }

            static inline void Write16(myos::common::uint16_t _port, myos::common::uint16_t _data) {
                __asm__ volatile("outw %0, %1" : : "a" (_data), "Nd" (_port));
            }
        };

        class Port32Bit : public Port {
        public:
            Port32Bit(myos::common::uint16_t portnumber);
            ~Port32Bit();
            virtual void Write(myos::common::uint32_t data);
            virtual myos::common::uint32_t Read();
        protected:
            static inline myos::common::uint32_t Read32(myos::common::uint16_t _port) {
                myos::common::uint32_t result;
                __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (_port));
                return result;
            }

            static inline void Write32(myos::common::uint16_t _port, myos::common::uint32_t _data) {
                __asm__ volatile("outl %0, %1" : : "a"(_data), "Nd" (_port));
            }
        };
    }
}

#endif