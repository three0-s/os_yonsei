#ifndef __KERNEL__
#define __KERNEL__

#include "core.hpp"

namespace os{
    enum class Mode{
        USER=0,
        KERNEL
    };

    class Kernel{
        private:
            static uint64_t cycle;
            static const std::vector<std::string> syscalls;
            static const std::vector<std::string> commands;
            static std::queue<os::Process*> ready_que;
            static std::queue<os::Process*> waiting_que;
            static os::Status kernel_status;

        public:
            static std::string cwd;
            void updateStatus(std::string command, os::Process* process);
            void newProcess(std::string pNmame, int ppid);
            void runProgram(std::string fname, int ppid);

    };

}


#endif