#ifndef __CORE__
#define __CORE__

#include "common.hpp"


namespace os{
    typedef std::vector<std::pair<uint8_t, uint8_t> > PageTable;
    const int VSIZE = 32;
    const int PSIZE = 16;

    class Process {
        public:
            std::string name; // process name
            int cur;
            int sleep_cycle;
            int n_run;
            int pid; // PID
            int ppid; // PPID
            char waiting_type; // S or W when paused
            os::VirtualMemory virtual_memory;
            os::PageTable page_table;


            void malloc(uint8_t n_alloc, os::PhysicalMemory* pMem, uint64_t cycle, std::string method);
            void frameRelease(uint8_t allocID, os::PhysicalMemory* pMem);
            void releaseAll(os::PhysicalMemory* pMem);

            Process(std::string _name, 
                    int _cur, 
                    int _sleep_cycle, 
                    int _n_run, 
                    int _pid, 
                    int _ppid, 
                    char _waiting_type,
                    int vsize, 
                    int psize):
                name(_name),
                cur(_cur), 
                sleep_cycle(_sleep_cycle),
                n_run(_n_run), 
                pid(_pid), 
                ppid(_ppid), 
                waiting_type(_waiting_type), 
                virtual_memory(vsize), 
                page_table(vsize, std::pair<uint8_t, uint8_t>(-1, -1)){}
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

    class Memory{
        public:
            uint8_t m_size;
            Memory(int size): m_size(size){}
    };

    class VirtualMemory: Memory{
        public:
            uint8_t pageID, allocID;
            std::vector<uint8_t> allocationIDs;
            std::vector<char> permissions;
            VirtualMemory(int vsize): Memory(vsize), pageID(0), allocID(0), allocationIDs(vsize, -1), permissions(vsize, 'W'){}
            VirtualMemory(const VirtualMemory&  vm);
    };

    class PhysicalMemory: Memory{
        public:
            std::vector<uint64_t> order;
            std::vector<uint8_t> frequency;
            std::queue<uint8_t> fifo;
            uint8_t base, limit;
            std::vector<bool> occupied;
            PhysicalMemory(int psize): Memory(psize), order(psize, 0), frequency(psize, 0), base(0), limit(psize), occupied(psize, false){}
            uint8_t frameAlloc(uint64_t tick, std::string method);
            void updateMemReg();
    };
}


#endif