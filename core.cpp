#include "core.hpp"

bool os::mfu(const std::pair<int,int> &a, const std::pair<int,int> &b){
    if (a.first > b.first) return true;
    if (a.first < b.first) return false;
    return (a.second < b.second);
}

bool os::lfu(const std::pair<int,int> &a, const std::pair<int,int> &b){
    if (a.first < b.first && a.first != -1) return true;
    if (a.first > b.first) return false;
    return (a.second < b.second && a.first != -1);
}

bool os::lru(const std::pair<int,int> &a, const std::pair<int,int> &b){
    if (a.first < b.first && a.first != -1) return true;
    if (a.first > b.first) return false;
    return (a.second < b.second && a.first != -1);
}

void os::Status::printStatus(std::string fname, os::PhysicalMemory* pMem){
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
        // 4. Physical Memory
        resultFile << "4. physical memory:\n";
        resultFile << "|";
        for (int i=0; i < os::PSIZE; i++){
            if (pMem->occupied[i].first==-1)
                resultFile << "-";
            else resultFile << pMem->occupied[i].first << "(" << pMem->occupied[i].second << ")";
            if (i%4 == 3) resultFile << "|";
            else    resultFile << " ";
        }
        resultFile << "\n";

        if (this->process_running != NULL) {
            // 5. Virtual Memory
            resultFile << "5. virtual memory:\n";
            resultFile << "|";
            for (int i=0; i < os::VSIZE; i++){
                if (process_running->virtual_memory->pageIDs[i]==-1)
                    resultFile << "-";
                else resultFile << process_running->virtual_memory->pageIDs[i];
                if (i%4 == 3) resultFile << "|";
                else    resultFile << " ";
            }
            resultFile << "\n";

            // 6. Page Table
            resultFile << "6. page table:\n";
            // first line
            resultFile << "|";
            for (int i=0; i < os::VSIZE; i++){
                if (process_running->page_table[i].second==-1)
                    resultFile << "-";
                else resultFile << process_running->page_table[i].second;
                if (i%4 == 3) resultFile << "|";
                else    resultFile << " ";
            }
            resultFile << "\n";
            // second line
            resultFile << "|";
            for (int i=0; i < os::VSIZE; i++){
                if (process_running->page_table[i].first==-1)
                    resultFile << "-";
                else resultFile << process_running->virtual_memory->permissions[i];
                if (i%4 == 3) resultFile << "|";
                else    resultFile << " ";
            }
            resultFile << "\n";
        }
        
        resultFile << "\n";
    }
    resultFile.close();
}


void os::PhysicalMemory::frameAlloc(uint64_t tick, std::string method, std::vector<int>* allocatedIDs, int n_alloc, bool searchAll){
    // update the base and limit register
    this->updateMemReg();
    for (int i=0; i < n_alloc && base != os::PSIZE; i++){
        allocatedIDs->push_back(this->base);
        // lru
        this->order[this->base]=tick;
        // lfu, mfu
        this->frequency[this->base]++;
        // fifo
        this->fifo.push(this->base);

        // update the base and limit register
        this->updateMemReg(base+1);
    }
    // out of memory
    if (base == os::PSIZE){
        // out of memory
        if (method=="lru"){
            std::vector<std::pair<int, int> > order_pairs;
            for (int i=0; i < order.size(); i++) order_pairs.push_back(std::pair<int, int> (order[i], i));
            std::sort(order_pairs.begin(), order_pairs.end(), os::lru);
            for (int i=0; allocatedIDs->size() < n_alloc; i++){
                bool duplicate=false;
                for (int id: *allocatedIDs) if (id == order_pairs[i].second) {duplicate=true; break;}
                if (duplicate) continue;
                allocatedIDs->push_back(order_pairs[i].second);
            }
        }
        else if (method=="fifo"){
            for (int i=fifo.front(); allocatedIDs->size() < n_alloc; fifo.pop(),i=fifo.front()) {
                bool duplicate=false;
                for (int id: *allocatedIDs) if (id == i) {duplicate=true; break;}
                if (duplicate) continue;
                allocatedIDs->push_back(i);
            }
        }
        else if (method=="lfu"){
            std::vector<std::pair<int, int> > frequency_pairs;
            for (int i=0; i < frequency.size(); i++) frequency_pairs.push_back(std::pair<int, int> (frequency[i], i));
            std::sort(frequency_pairs.begin(), frequency_pairs.end(), os::lfu);

            for (int i=0; allocatedIDs->size() < n_alloc; i++){
                bool duplicate=false;
                for (int id: *allocatedIDs) if (id == frequency_pairs[i].second) {duplicate=true; break;}
                if (duplicate) continue;
                allocatedIDs->push_back(frequency_pairs[i].second);
            }
        }
        else if (method=="mfu"){
            std::vector<std::pair<int, int> > frequency_pairs;
            for (int i=0; i < frequency.size(); i++) frequency_pairs.push_back(std::pair<int, int> (frequency[i], i));
            std::sort(frequency_pairs.begin(), frequency_pairs.end(), os::mfu);
            
            for (int i=0; allocatedIDs->size() < n_alloc; i++){
                bool duplicate=false;
                for (int id: *allocatedIDs) if (id == frequency_pairs[i].second) {duplicate=true; break;}
                if (duplicate) continue;
                allocatedIDs->push_back(frequency_pairs[i].second);
            }
        } 
        for (int victim: *allocatedIDs){
             // lru
            order[victim]=tick;
            // lfu, mfu
            frequency[victim]=1;
            // fifo
            this->fifo.push(victim);
        }

    }
}


void os::Process::pageTableUpdate(os::PhysicalMemory* pMem){
    for (int i=0; i < os::VSIZE; i++){
        if (virtual_memory->pageIDs[i] != -1){
            bool found = false;
            for (int j=0; j < os::PSIZE; j++){
                if (virtual_memory->pageIDs[i] == pMem->occupied[j].second){
                    page_table[i].second = j;
                    found = true;
                    break;
                }
            }
            if (!found)
                page_table[i].second = -1;
            
        }
    }

}

void os::PhysicalMemory::readMem(int addr, uint64_t tick){
    // lru
    this->order[addr]=tick;
    // lfu, mfu
    this->frequency[addr]++;
    // fifo
    this->fifo.push(addr);
}

void os::Process::malloc(int n_alloc, os::PhysicalMemory* pMem, uint64_t cycle, std::string method){
    for (int i=0; i < os::VSIZE-n_alloc; i++){
        this->virtual_memory->updateMemReg(i);
        if (this->virtual_memory->limit >= n_alloc) break;
    }
    virtual_memory = new VirtualMemory(*virtual_memory);
    std::vector<int> frameIDs;
    pMem->frameAlloc(cycle, method, &frameIDs, n_alloc);
    for (int i=0; i < n_alloc; i++){
        int idx = virtual_memory->base + i;
        page_table[idx].first = idx;
        page_table[idx].second = frameIDs[i];
        virtual_memory->permissions[idx]='W';

        // table update
        for (int j=0; j < os::VSIZE; j++){
            if (page_table[j].second == page_table[idx].second && j!= idx){
                page_table[j].second=-1;
            }
        }
        // Process ID
        pMem->occupied[page_table[idx].second].first = pid;
        // Page ID
        pMem->occupied[page_table[idx].second].second = this->pageID;

        virtual_memory->allocationIDs[idx] = allocID;
        virtual_memory->pageIDs[idx] = this->pageID++;
    }
    allocID++;
}


void os::Process::pageFaultHandler(int pNo, os::PhysicalMemory* pMem, uint64_t cycle, std::string method){
    std::vector<int> frameIDs;
    pMem->frameAlloc(cycle, method, &frameIDs, 1);

    page_table[pNo].first = pNo;
    page_table[pNo].second = frameIDs[0];
    virtual_memory->permissions[pNo]='W';
    // table update
    for (int j=0; j < os::VSIZE; j++){
        if (page_table[j].second == page_table[pNo].second && j!= pNo){
            page_table[j].second=-1;
        }
    }
    // Process ID
    pMem->occupied[page_table[pNo].second].first = pid;
    // Page ID
    pMem->occupied[page_table[pNo].second].second = virtual_memory->pageIDs[pNo];
}
            

void os::Process::protectionFaultHandler(int pNo, os::PhysicalMemory* pMem, std::queue<os::Process*>* readyQue, std::queue<os::Process*>* waitingQue){
    // permission change
    if (virtual_memory->permissions[pNo]=='R'){
        // Read only permission
        virtual_memory->permissions[pNo]='W'; 
        for (int j=0; j < readyQue->size(); j++){
            readyQue->front()->virtual_memory->permissions[pNo]='W';
            readyQue->push(readyQue->front());
            readyQue->pop();
        }
        for (int j=0; j < waitingQue->size(); j++){
            waitingQue->front()->virtual_memory->permissions[pNo]='W';
            waitingQue->push(waitingQue->front());
            waitingQue->pop();
        }
    } 
}
            


void os::VirtualMemory::updateMemReg(int st){
    bool hasHole=false;
    for (int i=st; i < os::VSIZE; i++){
        if (hasHole){
            if (allocationIDs[i]!=-1) break;
            this->limit++;
        }
        if (allocationIDs[i]==-1 && !hasHole){
            hasHole=true;
            this->base = i;
            this->limit = 1;
        }
    }
}


void os::PhysicalMemory::updateMemReg(int st){
    bool hasHole=false;
    for (int i=st; i < os::PSIZE; i++){
        if (hasHole){
            if (occupied[i].first!=-1) break;
            this->limit++;
        }
        if (occupied[i].first==-1 && !hasHole){
            hasHole=true;
            this->base = i;
            this->limit = 1;
        }
    }
    if (!hasHole){
        this->base=os::PSIZE;
        this->limit=0;
    }
}


os::VirtualMemory::VirtualMemory(const os::VirtualMemory& vm): Memory(vm.m_size), 
                    allocationIDs(vm.allocationIDs.size()),
                    permissions(vm.permissions.size()),
                    pageIDs(vm.pageIDs.size()){
    base = vm.base;
    for (int i=0; i < vm.m_size; i++){
        allocationIDs[i]=vm.allocationIDs[i];
        permissions[i]=vm.permissions[i];
        pageIDs[i]=vm.pageIDs[i];
    }
}


void os::Process::frameRelease(int aID, os::PhysicalMemory* pMem, std::queue<os::Process*>* readyQue, std::queue<os::Process*>* waitingQue){
   
    // permission change
    for (int i=0; i < os::VSIZE; i++){
        if (virtual_memory->allocationIDs[i]==aID){
            if (virtual_memory->permissions[i]=='R'){
                // Read only permission
                virtual_memory->permissions[i]='W'; 
                for (int j=0; j < readyQue->size(); j++){
                    readyQue->front()->virtual_memory->permissions[i]='W';
                    readyQue->push(readyQue->front());
                    readyQue->pop();
                }
                for (int j=0; j < waitingQue->size(); j++){
                    waitingQue->front()->virtual_memory->permissions[i]='W';
                    waitingQue->push(waitingQue->front());
                    waitingQue->pop();
                }
                
            } 
        }
    }
    bool cow = false;
    for (int i=0; i < os::VSIZE; i++){
        // release
        if (virtual_memory->allocationIDs[i]==aID){
            // parent owns the frame
            if (pid == pMem->occupied[page_table[i].second].first){
                // Process ID
                pMem->occupied[page_table[i].second].first=-1;
                // Page ID
                pMem->occupied[page_table[i].second].second=-1;

                
            } else{
                // CoW
                if (!cow) virtual_memory = new VirtualMemory(*virtual_memory);
                cow=true;
            }
            virtual_memory->base--;
            page_table[i].second=-1;
            page_table[i].first=-1;
            virtual_memory->allocationIDs[i]=-1;
            virtual_memory->pageIDs[i]=-1;
            pMem->updateMemReg();
        }
    }
}

void os::Process::releaseAll(os::PhysicalMemory* pMem, std::queue<os::Process*>* readyQue, std::queue<os::Process*>* waitingQue){
    // permission change
    for (int i=0; i < os::VSIZE; i++){
        if (virtual_memory->permissions[i]=='R'){
            // Read only permission
            virtual_memory->permissions[i]='W'; 
            for (int j=0; j < readyQue->size(); j++){
                readyQue->front()->virtual_memory->permissions[i]='W';
                readyQue->push(readyQue->front());
                readyQue->pop();
            }
            for (int j=0; j < waitingQue->size(); j++){
                waitingQue->front()->virtual_memory->permissions[i]='W';
                waitingQue->push(waitingQue->front());
                waitingQue->pop();
            }
        } 
    }
    bool cow=false;
    
    for (int i=0; i < os::VSIZE; i++){
        // parent owns the frame
        if (pid == pMem->occupied[page_table[i].second].first){
            // Process ID
            pMem->occupied[page_table[i].second].first=-1;
            // Page ID
            pMem->occupied[page_table[i].second].second=-1;
            
        } else{
            // CoW
            if (!cow) virtual_memory = new VirtualMemory(*virtual_memory);
            cow=true;
        }
        page_table[i].second=-1;
        page_table[i].first=-1;
        virtual_memory->allocationIDs[i]=-1;
        virtual_memory->pageIDs[i]=-1;
        pMem->updateMemReg();
    }
    virtual_memory->base=0;
}