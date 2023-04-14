#include "kernel.hpp"


uint64_t os::Kernel::cycle = 0;
os::Status os::Kernel::kernel_status= {os::Kernel::cycle, "kernel", "NONE", 
                                       NULL,
                                       os::Kernel::ready_que, 
                                       os::Kernel::waiting_que, 
                                       NULL, NULL};

// Allowed System Calls
const std::vector<std::string> os::Kernel::syscalls = {"sleep", "fork_and_exec", "wait", "exit"};
const std::vector<std::string> os::Kernel::commands = {"sleep", "fork_and_exec", "wait", "exit", "run"};


os::Kernel::Kernel(std::string _cwd){
    this-> cwd = _cwd;
}

void os::Kernel::updateStatus(std::string command, os::Process* process){
    if (command == "boot" || command == "system call"){
        this->kernel_status.process_new = process;
        this->kernel_status.mode = "kernel";
        if (this->kernel_status.process_running != NULL){
            this->ready_que.push(this->kernel_status.process_running);
            this->kernel_status.process_running = NULL;
        }
        this->kernel_status.command = command;
        this->kernel_status.printStatus();
    }
    else if (command =="schedule"){
        this->ready_que.push(this->kernel_status.process_new);
        this->kernel_status.process_new = NULL;
        this->kernel_status.process_running = this->ready_que.front();
        this->ready_que.pop();
        this->kernel_status.printStatus();
        this->kernel_status.mode = "user";
    }
    else if (command.find("sleep") != std::string::npos){
        return;
    }
    else if (command == "wait"){
        return;
    }
    else if (command == "exit"){
        return;
    }
    else if (command.find("run") != std::string::npos){
        return;
    }
    else{
        this->kernel_status.command=command;
        this->kernel_status.printStatus();
    }
    this->cycle++;
}

void os::Kernel::newProcess(std::string pName, int ppid){
    os::Process newProcess = {pName, ppid+1, ppid, '0'};
    // cycle+0]
    std::string command = ppid==0? "boot": "system call";
    this->updateStatus(command, &newProcess);

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

            // check if the program calls other programs
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