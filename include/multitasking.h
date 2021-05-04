
#ifndef __MYOS__MULTITASING_H
#define __MYOS__MULTITASING_H

#include "common/types.h"
#include "gdt.h"

namespace myos {
    struct CPUState {
        common::uint32_t eax, ebx, ecx, edx, esi, edi, ebp;

        common::uint32_t error, eip, cs, eflags, esp, ss;
    } __attribute__((packed));

    class Task {
        friend class TaskManger;
    public:
        Task(GlobalDescriptorTable* gdt, void entrypoint());
        ~Task();
    private:
        common::uint8_t stack[4096];
        CPUState* cpustate;
    };

    class TaskManger {
    public:
        TaskManger();
        ~TaskManger();
        bool AddTask(Task* task);
        CPUState* Schedule(CPUState* cpustate);
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
    };
}

#endif