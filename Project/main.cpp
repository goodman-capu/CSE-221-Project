//
//  main.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "CPU.cpp"
#include <sys/resource.h>

int main(int argc, const char * argv[]) {
    setpriority(PRIO_PROCESS, getpid(), -20);
    CPU::measureAll();
    return 0;
}
