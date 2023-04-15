#ifndef __CORE__
#define __CORE__

#include "common.hpp"


namespace os{

    class Process {
        public:
            std::string name; // process name
            int pid; // PID
            int ppid; // PPID
            char waiting_type; // S or W when paused
            Process(std::string _name, int _pid, int _ppid, char _waiting_type):
                name(_name), pid(_pid), ppid(_ppid), waiting_type(_waiting_type){}
    };

    class Status {
        public:
            uint64_t& cycle;
            std::string mode;
            std::string command;
            Process* process_running;
            std::queue<os::Process*>& process_ready;
            std::queue<os::Process*>& process_waiting;
            os::Process* process_new;
            os::Process* process_terminated;

        public:
            void printStatus(std::string fname);
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