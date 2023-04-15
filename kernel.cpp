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

// Allowed System Calls
const std::vector<std::string> os::Kernel::syscalls = {"sleep", "fork_and_exec", "wait", "exit"};
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

void os::Kernel::updateStatus(std::string command, os::Process* process){
    this->kernel_status.command = command;
    if (command == "boot" || command == "system call"){
        this->kernel_status.process_new = process;
        this->kernel_status.mode = "kernel";
        if (this->kernel_status.process_running != NULL){
            this->ready_que.push(this->kernel_status.process_running);
            this->kernel_status.process_running = NULL;
        }
        this->printKernelStatus();
        this->cycle++;
    }
    else if (command =="schedule"){
        this->ready_que.push(this->kernel_status.process_new);
        this->kernel_status.process_new = NULL;
        this->kernel_status.process_running = this->ready_que.front();
        this->ready_que.pop();
        this->printKernelStatus();
        this->kernel_status.mode = "user";
        this->cycle++;
    }
    else if (command.find("sleep") != std::string::npos){
        // cycle+0]
        this->printKernelStatus();
        this->kernel_status.mode = "kernel";
        this->cycle++;

        // cycle+1] system call
        this->kernel_status.command="system call";
        int SLEEP_CNT = std::stoi(command.substr(6));
        if (this->kernel_status.process_running != NULL){
            this->kernel_status.process_running->waiting_type='S';
            this->waiting_que.push(this->kernel_status.process_running);
            this->kernel_status.process_running = NULL;
        }
        if (SLEEP_CNT==1){
            os::Process* restored = this->waiting_que.front();
            restored->waiting_type = '0';
            this->waiting_que.pop();
            this->ready_que.push(restored);
        }
        this->printKernelStatus();
        this->cycle++;      

        // cycle+2] schedule
        this->kernel_status.command="schedule";
        while(SLEEP_CNT--){
            if (SLEEP_CNT==0){
                os::Process* restored = this->waiting_que.front();
                restored->waiting_type = '0';
                this->waiting_que.pop();
                this->ready_que.push(restored);
                this->kernel_status.process_running = this->ready_que.front();
                this->ready_que.pop();
            }
            this->printKernelStatus();
            this->cycle++;      
        }
        this->kernel_status.mode = "user";
    }
    else if (command == "wait"){
        // cycle+0]
        this->printKernelStatus();
        this->cycle++;
        this->kernel_status.mode="kernel";

        this->kernel_status.command="system call";
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
        }
        this->printKernelStatus();
        this->cycle++;
        
        // cycle+2] schedule
        if (!this->ready_que.empty()){
            this->kernel_status.command = "schedule";
            this->kernel_status.process_running = this->ready_que.front();
            this->ready_que.pop();
            this->printKernelStatus();
            this->kernel_status.mode = "user";
            this->cycle++;
        }
        else{
            // idle
            this->kernel_status.command = "idle";
            this->printKernelStatus();
            this->cycle++;
        }
    }
    else if (command == "exit"){
        // cycle+0] user-mode
        this->printKernelStatus();
        this->kernel_status.mode="kernel";
        this->cycle++;

        // cycle+1] kernel mode; system call
        this->kernel_status.command = "system call";
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
        this->cycle++;

        if (this->ready_que.empty() && this->waiting_que.empty() && this->kernel_status.process_running==NULL)
            return;
        
        // cycle+2] schedule
        if (!this->ready_que.empty()){
            this->kernel_status.command = "schedule";
            this->kernel_status.process_running = this->ready_que.front();
            this->ready_que.pop();
            this->printKernelStatus();
            this->kernel_status.mode = "user";
            this->cycle++;
        }
        else{
            // idle
            this->kernel_status.command = "idle";
            this->printKernelStatus();
            this->cycle++;
        }
        
    }
    else if (command.find("run") != std::string::npos){
        int RCYCLE = std::stoi(command.substr(4));
        for (int i=0; i < RCYCLE; i++){
            this->printKernelStatus();
            this->cycle++;
        }
    }
    else{
        this->printKernelStatus();
        this->cycle++;
    }
    
}

void os::Kernel::newProcess(std::string pName, int ppid){
    os::Process* newProcess = new Process(pName, ppid+1, ppid, '0');
    // cycle+0]
    std::string command = ppid==0? "boot": "system call";
    this->updateStatus(command, newProcess);

    // cycle+1] schedule
    command = "schedule";
    this->updateStatus(command, NULL);
}

void os::Kernel::runProgram(std::string pname, int ppid){
    std::queue<std::string> otherPrograms;
    std::string pPath = this->cwd + "/" + pname;
    std::ifstream program;

    program.open(pPath, std::ios_base::in);
    if(program.is_open()){
        newProcess(pname, ppid);
        std::string command;
        //read until the end of the program
        while(!program.eof()){ 
            std::getline(program, command);    
            this->updateStatus(command, NULL);

            // check if the program calls child process
            if (command.find(this->commands[1])!=std::string::npos){
                command = command.substr(commands[1].length()+1);
                newProcess(command, ppid+1);
                otherPrograms.push(command);
            }
        }
    }
    while(!otherPrograms.empty()){
        this->runProgram(otherPrograms.front(), ppid+1);
        otherPrograms.pop();
    }
}