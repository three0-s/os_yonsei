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
    std::string pChange = argv[2];
    os::Kernel::cwd = dir;
    os::Kernel::method = pChange;
    os::Kernel simulator;
    simulator.run();
    return 0;
}