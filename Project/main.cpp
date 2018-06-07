//
//  main.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "CPU.cpp"
#include "Memory.cpp"
#include "FileSystem.cpp"

int main(int argc, const char * argv[]) {
    CPU::measure_all();
    Memory::measure_all();
    FileSystem::measure_all();
    return 0;
}
