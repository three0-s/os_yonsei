#ifndef __CORE__
#define __CORE__

#include "common.hpp"


namespace os{
    typedef std::vector<std::pair<int, int> > PageTable;
    const int VSIZE = 32;
    const int PSIZE = 16;
    class Memory{
        public:
            int m_size;
            Memory(int size): m_size(size){}
    };

    bool mfu(const std::pair<int,int> &a, const std::pair<int,int> &b);
    bool lfu(const std::pair<int,int> &a, const std::pair<int,int> &b);
    bool lru(const std::pair<int,int> &a, const std::pair<int,int> &b);

    class VirtualMemory: Memory{
        public:
            int base, limit;
            std::vector<int> allocationIDs, pageIDs;
            std::vector<char> permissions;
            void updateMemReg(int st);
            VirtualMemory(int vsize): Memory(vsize), base(0), limit(vsize), allocationIDs(vsize, -1), pageIDs(vsize, -1), permissions(vsize, 'W'){}
            VirtualMemory(const VirtualMemory&  vm);
    };

    class PhysicalMemory: Memory{
        public:
            std::vector<uint64_t> order;
            std::vector<int> frequency;
            std::queue<int> fifo;
            int base, limit;
            std::vector<std::pair<int, int>> occupied;
            PhysicalMemory(int psize): Memory(psize), order(psize, -1), frequency(psize, 0), base(0), limit(psize), occupied(psize, std::pair<int, int>(-1, -1)){}
            void frameAlloc(uint64_t tick, std::string method, std::vector<int>* allocatedIDs, int n_alloc, bool searchAll=true);
            void readMem(int addr, uint64_t tick);
            void updateMemReg(int st=0);
    };

    class Process {
        public:
            std::string name; // process name
            int cur;
            int sleep_cycle, pageID, allocID;
            int n_run;
            int pid; // PID
            int ppid; // PPID
            char waiting_type; // S or W when paused
            os::VirtualMemory* virtual_memory;
            os::PageTable page_table;

            void pageFaultHandler(int pNo, os::PhysicalMemory* pMem, uint64_t cycle, std::string method);
            void pageTableUpdate(os::PhysicalMemory* pMem);
            void protectionFaultHandler(int pNo, os::PhysicalMemory* pMem, std::queue<os::Process*>* readyQue, std::queue<os::Process*>* waitingQue);
            void malloc(int n_alloc, os::PhysicalMemory* pMem, uint64_t cycle, std::string method);
            void frameRelease(int aID, os::PhysicalMemory* pMem, std::queue<os::Process*>* readyQue, std::queue<os::Process*>* waitingQue);
            void releaseAll(os::PhysicalMemory* pMem, std::queue<os::Process*>* readyQue, std::queue<os::Process*>* waitingQue);

            Process(std::string _name, 
                    int _cur, 
                    int _sleep_cycle, 
                    int _n_run, 
                    int _pid, 
                    int _ppid, 
                    char _waiting_type,
                    int vsize,
                    VirtualMemory* vm, 
                    int psize):
                name(_name),
                cur(_cur), 
                allocID(0), 
                pageID(0),
                sleep_cycle(_sleep_cycle),
                n_run(_n_run), 
                pid(_pid), 
                ppid(_ppid), 
                waiting_type(_waiting_type), 
                virtual_memory(vm), 
                page_table(vsize, std::pair<int, int>(-1, -1)){}
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
            void printStatus(std::string fname, os::PhysicalMemory* pMem);
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