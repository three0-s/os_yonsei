/*
    23.4.13
    Yonsei Univ. CSI3101
        Operating System 
            Project 2.
    Lim, Yewon (2019145010)
    ga06033@yonsei.ac.kr
*/
#include "common.hpp"
#include "core.hpp"
#include "kernel.hpp"

int main(int argc, char** argv) {
    std::string dir = argv[1];
    os::Kernel::cwd = dir;
    os::Kernel simulator;
    simulator.runProgram("init", 0);
    return 0;
}