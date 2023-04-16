#ifndef __KERNEL__
#define __KERNEL__

#include "core.hpp"

namespace os{
    class Kernel{
        private:
            static uint64_t cycle;
            static const std::vector<std::string> commands;
            static std::queue<os::Process*> ready_que;
            static std::queue<os::Process*> waiting_que;
            static os::Status kernel_status;

        public:
            static std::string cwd;
            void updateStatus(std::string command, os::Process* process);
            void newProcess(std::string pNmame, int ppid);
            void printKernelStatus();
            void run();
            void run_();
            std::string commandSchedule();
            Kernel();
    };

}

#endif