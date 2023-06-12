#include "kernel.hpp"


// =============================================================================
/* Static variables init */
std::queue<os::Process*> os::Kernel::ready_que;
std::queue<os::Process*> os::Kernel::waiting_que;
std::string os::Kernel::cwd = "";
uint64_t os::Kernel::cycle = 0;
os::Status os::Kernel::kernel_status= {os::Kernel::cycle, "kernel", "NONE", 
                                       NULL,
                                       os::Kernel::ready_que, 
                                       os::Kernel::waiting_que, 
                                       NULL, NULL};
// =============================================================================

os::Kernel::Kernel(): physical_memory(os::PSIZE), fault(false){
    std::ofstream resultFile("result", std::ios_base::trunc);
    if (resultFile.is_open()){
        resultFile << "";
    }
    resultFile.close();
}

void os::Kernel::printKernelStatus(){
    this->kernel_status.printStatus("result");
}

std::string os::Kernel::commandSchedule(){
    if(this->kernel_status.mode=="kernel"){
        // boot
        if (this->cycle==0) return "boot";
        // system call
        if (this->kernel_status.process_running!=NULL){
            // memory fault
            if(this->fault) return "fault";
            return "system call";
        }
            
        // schedule
        if (!this->ready_que.empty() || this->kernel_status.process_new!=NULL) return "schedule";
        else    return "idle";
    }
    else{
        if (this->kernel_status.process_running->n_run != 0){
            return "prev";
        }
        // there should be at least one user process running
        std::ifstream pFile;
        std::string command;

        // resume from the previous point
        pFile.open(this->cwd + "/" + this->kernel_status.process_running->name, std::ios_base::in);
        for (int i=0; i <= this->kernel_status.process_running->cur; i++){
            std::getline(pFile, command);
        }
        this->kernel_status.process_running->cur++;
        return command;
    }
}

void os::Kernel::run(){
    int pid=1, ppid=0;
    os::Process* program = new Process("init", 0, 0, 0, pid++, ppid, '0', os::VSIZE, os::PSIZE);
    std::queue<Process*> activated;
    std::string userOperation="", cmd;
   
    while(true){
        // terminate the simulator
        if (this->cycle>0 && this->ready_que.empty() && this->waiting_que.empty() && this->kernel_status.process_running==NULL
            && this->kernel_status.process_new==NULL)
            break;

        int queSize =  this->waiting_que.size();
        // // // sleep cycle update
        // // for (int i=0; i < queSize; i++){
        // //     if (this->waiting_que.front()->waiting_type=='S')
        // //         this->waiting_que.front()->sleep_cycle--;
        // //     this->waiting_que.push(this->waiting_que.front());
        // //     this->waiting_que.pop();
        // // }
        if (userOperation!="exit"){
            // for (int i=0; i < queSize; i++){
            //     // pop process of which sleep cycle ends
            //     if (this->waiting_que.front()->sleep_cycle==0 && this->waiting_que.front()->waiting_type=='S'){
            //         this->waiting_que.front()->waiting_type='0';
            //         activated.push(this->waiting_que.front());
            //         this->waiting_que.pop();
            //         continue;
            //     }
            //     this->waiting_que.push(this->waiting_que.front());
            //     this->waiting_que.pop();
            // }
            while(!activated.empty()){
                this->ready_que.push(activated.front());
                activated.pop();
            }

        }
        // command
        cmd = this->commandSchedule();
        if (cmd == "prev")  cmd = userOperation;
        this->kernel_status.command = cmd;

        // kernel mode
        if (cmd == "boot"){
            this->kernel_status.process_new = program;
            this->printKernelStatus();
        }
        else if (cmd == "schedule"){
            if (this->kernel_status.process_new!=NULL){
                this->ready_que.push(this->kernel_status.process_new);
                this->kernel_status.process_new = NULL;
            }
            this->kernel_status.process_running = this->ready_que.front();
            this->ready_que.pop();
            this->printKernelStatus();
            this->kernel_status.mode="user";
        }
        else if (cmd == "idle") this->printKernelStatus(); // do nothing
        else if (cmd == "system call"){
            // sleep
            if (userOperation.find("sleep") != std::string::npos){
                int SLEEP_CNT = std::stoi(userOperation.substr(6));
                Process* currentProcess = this->kernel_status.process_running;
                this->kernel_status.process_running=NULL;
                if (--SLEEP_CNT!=0){
                    currentProcess->waiting_type='S';
                    currentProcess->sleep_cycle=SLEEP_CNT;
                    this->waiting_que.push(currentProcess);
                }
                else{
                    this->ready_que.push(currentProcess);
                }
                
                this->printKernelStatus();
            }
            // wait
            else if(userOperation == "wait"){
                // if child process exists
                bool haveChild=false;
                for (int i=0; i < this->ready_que.size(); i++){
                    if(this->ready_que.front()->ppid == this->kernel_status.process_running->pid){
                        haveChild=true;
                    }
                    ready_que.push(ready_que.front());
                    ready_que.pop();
                }
                for (int i=0; i < this->waiting_que.size(); i++){
                    if(this->waiting_que.front()->ppid == this->kernel_status.process_running->pid){
                        haveChild=true;
                    }
                    waiting_que.push(waiting_que.front());
                    waiting_que.pop();
                }
                if (haveChild){
                    this->kernel_status.process_running->waiting_type='W';
                    this->waiting_que.push(this->kernel_status.process_running);
                    this->kernel_status.process_running=NULL;
                }
                else{
                    this->ready_que.push(this->kernel_status.process_running);
                    this->kernel_status.process_running=NULL;
                }
                this->printKernelStatus();
            }
            // exit
            else if(userOperation == "exit"){
                this->kernel_status.process_terminated = this->kernel_status.process_running;
                this->kernel_status.process_terminated->releaseAll(&(this->physical_memory));
                this->kernel_status.process_running=NULL;
                queSize =  this->waiting_que.size();
                // pop process of which sleep cycle ends
                for (int i=0; i < queSize; i++){
                    if (this->waiting_que.front()->sleep_cycle==0 && this->waiting_que.front()->waiting_type=='S'){
                        this->waiting_que.front()->waiting_type='0';
                        activated.push(this->waiting_que.front());
                        this->waiting_que.pop();
                        continue;
                    }
                    if (this->waiting_que.front()->pid == this->kernel_status.process_terminated->ppid && this->waiting_que.front()->waiting_type=='W'){
                        this->waiting_que.front()->waiting_type='0';
                        activated.push(this->waiting_que.front());
                        this->waiting_que.pop();
                        continue;
                    }
                    this->waiting_que.push(this->waiting_que.front());
                    this->waiting_que.pop();
                }
                while(!activated.empty()){
                    this->ready_que.push(activated.front());
                    activated.pop();
                }
        
                this->printKernelStatus();
                delete this->kernel_status.process_terminated;
                this->kernel_status.process_terminated=NULL;
            }
            // fork_and_exec
            else if(userOperation.find("fork_and_exec") != std::string::npos){
                std::string pName = userOperation.substr(14);
                program = new Process(pName, 0, 0, 0, pid++, ppid, '0', os::VSIZE, os::PSIZE);

                // copy parents' virtual memory and page table
                program->virtual_memory = this->kernel_status.process_running->virtual_memory;
                program->page_table = this->kernel_status.process_running->page_table;

                // change permission 
                for (int i=0; i < os::VSIZE; i++){
                    program->virtual_memory.permissions[i]='R';
                    this->kernel_status.process_running->virtual_memory.permissions[i]='R';
                }

                this->kernel_status.process_new = program;
                
                this->ready_que.push(this->kernel_status.process_running);
                this->kernel_status.process_running = NULL;
                
                this->printKernelStatus();
            }

            // memory_allocate
            else if(userOperation.find("memory_allocate") != std::string::npos){
                uint8_t n_alloc = (uint8_t)std::stoi(userOperation.substr(strlen("memory_allocate")+1));
                
                for (uint8_t i=0; i < n_alloc; i++){
                    uint8_t idx = this->kernel_status.process_running->virtual_memory.pageID + i;
                    this->kernel_status.process_running->page_table[idx].first = idx;
                    this->kernel_status.process_running->page_table[idx].second = this->physical_memory.frameAlloc(this->cycle, os::Kernel::method);
                    this->kernel_status.process_running->virtual_memory.allocationIDs[idx] = this->kernel_status.process_running->virtual_memory.allocID;
                }

                this->kernel_status.process_running->virtual_memory.pageID+= n_alloc;
                this->kernel_status.process_running->virtual_memory.allocID++;

                this->kernel_status.process_ready.push(this->kernel_status.process_running);
                this->kernel_status.process_running = NULL;

                this->printKernelStatus();

            }
            // memory_release
            else if(userOperation.find("memory_release") != std::string::npos){
                uint8_t allocID = (uint8_t)std::stoi(userOperation.substr(strlen("memory_release")+1));
                this->kernel_status.process_running->frameRelease(allocID, &(this->physical_memory));
                this->kernel_status.process_ready.push(this->kernel_status.process_running);
                this->kernel_status.process_running = NULL;

                this->printKernelStatus();
            }
            
        }
        else if (cmd == "fault"){
            // memory_read page fault
            if(userOperation.find("memory_read") != std::string::npos){
                
                
                this->ready_que.push(this->kernel_status.process_running);
                this->kernel_status.process_running = NULL;
                this->printKernelStatus();
            }

            // memory_write fault ( protection || page )
            else if(userOperation.find("memory_write") != std::string::npos){
                // Parent process should have ppid of 0
                if (this->kernel_status.process_running->ppid==0){
                    
                // Child process
                } else{

                }
                this->ready_que.push(this->kernel_status.process_running);
                this->kernel_status.process_running = NULL;
                
                this->printKernelStatus();
            }
        }
        // user mode
        else{
            // memory read
            if(cmd.find("memory_read") != std::string::npos){
                uint8_t pageID = (uint8_t)std::stoi(cmd.substr(strlen("memory_read")+1));
                this->printKernelStatus(); 
                if (this->kernel_status.process_running->page_table[pageID].second == -1){
                    this->kernel_status.mode="kernel";
                    this->fault=true;
                }
            }
            // memory write
            else if(cmd.find("memory_write") != std::string::npos){
                uint8_t pageID = (uint8_t)std::stoi(cmd.substr(strlen("memory_write")+1));
                this->printKernelStatus(); 
                // 'W' permission -  page fault
                if (this->kernel_status.process_running->virtual_memory.permissions[pageID]=='W'){
                    if (this->kernel_status.process_running->page_table[pageID].second == -1){
                        this->kernel_status.mode="kernel";
                        this->fault=true;
                    }
                // 'R' permission -  protection fault
                } else{
                    this->kernel_status.mode="kernel";
                    this->fault=true;
                }
            }
            // mode switch
            // "sleep", "fork_and_exec", "wait", "exit", "memory allocate", "memory release"
            else{
                this->printKernelStatus(); 
                this->kernel_status.mode="kernel";
                ppid = this->kernel_status.process_running->pid;
            }
            userOperation=cmd;
        }

        this->cycle++;
    }
}
