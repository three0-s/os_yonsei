#ifndef __KERNEL__
#define __KERNEL__

#include "core.hpp"

namespace os{
    class Kernel{
        private:
            static uint64_t cycle;
            // static const std::vector<std::string> commands;
            static std::queue<os::Process*> ready_que;
            static std::queue<os::Process*> waiting_que;
            static os::Status kernel_status;
            os::PhysicalMemory physical_memory;
            bool fault;
        public:
            static std::string cwd;
            static std::string method;
            void printKernelStatus();
            void run();
            std::string commandSchedule();
            Kernel();
    };

}

#endif