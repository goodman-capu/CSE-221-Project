//
//  CPU.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "Measurer.cpp"
#include <x86intrin.h>

static int loop = 10000;

class CPU {
public:
    static void measureAll() {
        Measurer::measure(timeOverhead, "Time Overhead");
        Measurer::measure(loopOverhead, "Loop Overhead");
        for (int i = 0; i <= 7; ++i) {
            function<double()> func = [&i](){return procedureCallOverhead(i);};
            Measurer::measure(func, "Procedure Call Overhead (" + to_string(i) + " params)");
        }
    }
    
private:
    static double timeOverhead() {
        uint64_t start, end;
        double sum = 0;
        
        for (int i = 0; i < loop; ++i) {
            start = __rdtsc();
            end = __rdtsc();
            sum += (end - start);
        }
        
        return sum / loop;
    }
    
    static double loopOverhead() {
        uint64_t start, end;
        double sum = 0;
        int loopTime = 10000;
        
        for (int i = 0; i < loop; ++i) {
            start = __rdtsc();
            for (int j = 0; j < loopTime; ++j) {
                // Do nothing
            }
            end = __rdtsc();
            sum += (double)(end - start) / loopTime;
        }
        
        return sum / loop;
    }
    
    static void procedureCall() {
        // Do nothing
    }

    static void procedureCall(int a, ...) {
        // Do nothing
    }
    
    static double procedureCallOverhead(int paraNum) {
        uint64_t start, end;
        double sum = 0;
        
        function<void()> callFunc = [](){procedureCall();};
        switch (paraNum) {
            case 0:
                callFunc = [](){procedureCall();};
                break;
            case 1:
                callFunc = [](){procedureCall(0);};
                break;
            case 2:
                callFunc = [](){procedureCall(0, 0);};
                break;
            case 3:
                callFunc = [](){procedureCall(0, 0, 0);};
                break;
            case 4:
                callFunc = [](){procedureCall(0, 0, 0, 0);};
                break;
            case 5:
                callFunc = [](){procedureCall(0, 0, 0, 0, 0);};
                break;
            case 6:
                callFunc = [](){procedureCall(0, 0, 0, 0, 0, 0);};
                break;
            case 7:
                callFunc = [](){procedureCall(0, 0, 0, 0, 0, 0, 0);};
                break;
            default:
                break;
        }
        
        for (int i = 0; i < loop; ++i) {
            start = __rdtsc();
            callFunc();
            end = __rdtsc();
            sum += (end - start);
        }
        
        return sum / loop;
    }
};
