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

// Allowed Commands
const std::vector<std::string> os::Kernel::commands = {"sleep", "fork_and_exec", "wait", "exit", "run"};
// =============================================================================

os::Kernel::Kernel(){
    std::ofstream resultFile(this->cwd+"/result", std::ios_base::trunc);
    if (resultFile.is_open()){
        resultFile << "";
    }
    resultFile.close();
}

void os::Kernel::printKernelStatus(){
    this->kernel_status.printStatus(this->cwd+"/result");
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
        // sleep cycle check
        for (int i=0; i < queSize; i++){
            if (this->waiting_que.front()->waiting_type=='S')
                this->waiting_que.front()->sleep_cycle--;
            // pop process of which sleep cycle ends
            if (this->waiting_que.front()->sleep_cycle==0 && this->waiting_que.front()->waiting_type=='S'){
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
                currentProcess->waiting_type='S';
                currentProcess->sleep_cycle=SLEEP_CNT;
                this->waiting_que.push(currentProcess);
                this->kernel_status.process_running = NULL;
            
                if (--currentProcess->sleep_cycle==0){
                    os::Process* restored = this->waiting_que.front();
                    restored->waiting_type = '0';
                    this->waiting_que.pop();
                    this->ready_que.push(restored);
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
                
                // check if the parent process is waiting
                if (!this->waiting_que.empty()){
                    if (this->waiting_que.front()->pid == this->kernel_status.process_terminated->ppid){
                        os::Process* parent = this->waiting_que.front();
                        parent-> waiting_type='0';
                        this->ready_que.push(parent);
                        this->waiting_que.pop();
                    }
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
        }
        // user mode
        else{
            // "run"
            // user mode operation
            if (cmd.find("run") != std::string::npos){
                int RCYCLE = std::stoi(cmd.substr(4));
                // init RCYCLE of the process
                if (this->kernel_status.process_running->n_run == 0 && userOperation!=cmd)
                    this->kernel_status.process_running->n_run=RCYCLE;
                
                this->kernel_status.process_running->n_run--;
                this->printKernelStatus();
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
