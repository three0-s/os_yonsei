#include "core.hpp"

void os::Status::printStatus(std::string fname){
    std::ofstream resultFile(fname, std::ios_base::app);
    if (resultFile.is_open()){
        // 0. 몇번째 cycle인지
        resultFile << "[cycle #"<< this->cycle << "]\n";
        // 1. 현재 실행 모드 (user or kernel)
        resultFile << "1. mode: "<< this->mode.c_str() << "\n";
        // 2. 현재 실행 명령어
        resultFile << "2. command: "<< this->command.c_str() << "\n";
        // 3. 현재 실행중인 프로세스의 정보. 없을 시 none 출력
        if (this->process_running == NULL) {
            resultFile << "3. running: none\n";
        } else {
            Process* i = this->process_running;
            resultFile << "3. running: "<< i->pid << "(" << i->name.c_str() << ", " << i->ppid << ")\n";
        }
        // 4. 현재 ready 상태인 프로세스의 정보. 왼쪽에 있을 수록 먼저 queue에 들어온 프로세스이다.
        if (this->process_ready.size() == 0) {
            resultFile << "4. ready: none\n";
        } else {
            resultFile << "4. ready:";
            for (int i = 0; i < this->process_ready.size(); ++i) { // 공백 한칸으로 구분
                resultFile << " " << this->process_ready.front()->pid;
                this->process_ready.push(this->process_ready.front());
                this->process_ready.pop();
            }
            resultFile << "\n";
        }
        // 5. 현재 waiting 상태인 프로세스의 정보. 왼쪽에 있을 수록 먼저 waiting이 된 프로세스이다.
        if (this->process_waiting.size() == 0) {
            resultFile << "5. waiting: none\n";
        } else {
            resultFile << "5. waiting:";
            for (int i = 0; i < this->process_waiting.size(); ++i) { // 공백 한칸으로 구분
                resultFile << " " << this->process_waiting.front()->pid << "(" << this->process_waiting.front()->waiting_type << ")";
                this->process_waiting.push(this->process_waiting.front());
                this->process_waiting.pop();
            }
            resultFile << "\n";
        }
        // 6. New 상태의 프로세스
        if (this->process_new == NULL) {
            resultFile << "6. new: none\n";
        } else {
            Process* i = this->process_new;
            resultFile << "6. new: "<< i->pid << "(" << i->name.c_str() << ", " << i->ppid << ")\n";
        }
        // 7. Terminated 상태의 프로세스
        if (this->process_terminated == NULL) {
            resultFile << "7. terminated: none\n";
        } else {
            Process* i = this->process_terminated;
            resultFile << "7. terminated: "<< i->pid << "("<<i->name.c_str() << ", " << i->ppid << ")\n";
        }
        // 매 cycle 간의 정보는 두번의 개행으로 구분
        resultFile << "\n";
    }
    resultFile.close();
}
