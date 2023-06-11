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

os::Kernel::Kernel(){
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
        if (this->kernel_status.process_running!=NULL) return "system call";
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
    os::Process* program = new Process("init", 0, 0, 0, pid++, ppid, '0');
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
                program = new Process(pName, 0, 0, 0, pid++, ppid, '0');
                this->kernel_status.process_new = program;
                if (this->kernel_status.process_running != NULL){
                    this->ready_que.push(this->kernel_status.process_running);
                    this->kernel_status.process_running = NULL;
                }
                this->printKernelStatus();
            }
            // memory_allocate
            else if(userOperation.find("memory_allocate") != std::string::npos){
                
            }
            // memory_release
            else if(userOperation.find("memory_release") != std::string::npos){
                
            }

            // memory_read fault
            else if(userOperation.find("memory_read") != std::string::npos){
                
            }

            // memory_write fault
            else if(userOperation.find("memory_write") != std::string::npos){
                
            }
            
        }
        // user mode
        else{
            // memory read
            if(cmd.find("memory_read") != std::string::npos){
                // MEM READ
            }
            // memory write
            else if(cmd.find("memory_write") != std::string::npos){
                // MEM WRITE
            }
            // mode switch
            // "sleep", "fork_and_exec", "wait", "exit"
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
