#include "core.hpp"

void os::Status::printStatus(){
    // 0. 몇번째 cycle인지
    printf("[cycle #%d]\n", this->cycle);

    // 1. 현재 실행 모드 (user or kernel)
    printf("1. mode: %s\n", this->mode.c_str());

    // 2. 현재 실행 명령어
    printf("2. command: %s\n", this->command.c_str());

    // 3. 현재 실행중인 프로세스의 정보. 없을 시 none 출력
    if (this->process_running == NULL) {
        printf("3. running: none\n");
    } else {
        Process* i = this->process_running;
        printf("3. running: %d(%s, %d)\n", i->pid, i->name.c_str(), i->ppid);
    }

     // 4. 현재 ready 상태인 프로세스의 정보. 왼쪽에 있을 수록 먼저 queue에 들어온 프로세스이다.
    if (this->process_ready.size() == 0) {
        printf("4. ready: none\n");
    } else {
        printf("4. ready:");
        for (int i = 0; i < this->process_ready.size(); ++i) { // 공백 한칸으로 구분
            printf(" %d", this->process_ready.front()->pid);
            this->process_ready.push(this->process_ready.front());
            this->process_ready.pop();
        }
        printf("\n");
    }

     // 5. 현재 waiting 상태인 프로세스의 정보. 왼쪽에 있을 수록 먼저 waiting이 된 프로세스이다.
    if (this->process_waiting.size() == 0) {
        printf("5. waiting: none\n");
    } else {
        printf("5. waiting:");
        for (int i = 0; i < this->process_waiting.size(); ++i) { // 공백 한칸으로 구분
            printf(" %d(%c)", this->process_waiting.front()->pid, this->process_waiting.front()->waiting_type);
            this->process_waiting.push(this->process_waiting.front());
            this->process_waiting.pop();
        }
        printf("\n");
    }

     // 6. New 상태의 프로세스
    if (this->process_new == NULL) {
        printf("6. new: none\n");
    } else {
        Process* i = this->process_new;
        printf("6. new: %d(%s, %d)\n", i->pid, i->name.c_str(), i->ppid);
    }

     // 7. Terminated 상태의 프로세스
    if (this->process_terminated == NULL) {
        printf("7. terminated: none\n");
    } else {
        Process* i = this->process_terminated;
        printf("7. terminated: %d(%s, %d)\n", i->pid, i->name.c_str(), i->ppid);
    }
    // 매 cycle 간의 정보는 두번의 개행으로 구분
    printf("\n");
}
