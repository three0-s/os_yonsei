#ifndef __CORE__
#define __CORE__

#include "common.hpp"


namespace os{

    struct Process {
        std::string name; // process name
        int pid; // PID
        int ppid; // PPID
        char waiting_type; // S or W when paused
    };

    class Status {
        public:
            uint64_t cycle;
            std::string mode;
            std::string command;
            Process* process_running;
            std::queue<os::Process*> process_ready;
            std::queue<os::Process*> process_waiting;
            os::Process* process_new;
            os::Process* process_terminated;

        public:
            void printStatus();
            Status(uint64_t& _cycle, 
                   std::string _mode, 
                   std::string _command, 
                   os::Process* process, 
                   std::queue<os::Process*>& ready, 
                   std::queue<os::Process*>& waiting,
                   os::Process* new_ps,
                   os::Process* term_ps 
            ): cycle(_cycle), 
               mode(_mode), 
               command(_command), 
               process_running(process),
               process_ready(ready),
               process_waiting(waiting),
               process_new(new_ps),
               process_terminated(term_ps){}
    };
}


#endif