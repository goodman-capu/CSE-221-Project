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
        
        for (int i = 0; i < loop; ++i) {
            switch (paraNum) {
                case 1:
                    start = __rdtsc();
                    procedureCall(0);
                    end = __rdtsc();
                    break;
                case 2:
                    start = __rdtsc();
                    procedureCall(0, 0);
                    end = __rdtsc();
                    break;
                case 3:
                    start = __rdtsc();
                    procedureCall(0, 0, 0);
                    end = __rdtsc();
                    break;
                case 4:
                    start = __rdtsc();
                    procedureCall(0, 0, 0, 0);
                    end = __rdtsc();
                    break;
                case 5:
                    start = __rdtsc();
                    procedureCall(0, 0, 0, 0, 0);
                    end = __rdtsc();
                    break;
                case 6:
                    start = __rdtsc();
                    procedureCall(0, 0, 0, 0, 0, 0);
                    end = __rdtsc();
                    break;
                case 7:
                    start = __rdtsc();
                    procedureCall(0, 0, 0, 0, 0, 0, 0);
                    end = __rdtsc();
                    break;
                default: // 0 params
                    start = __rdtsc();
                    procedureCall();
                    end = __rdtsc();
                    break;
            }
            sum += (end - start);
        }
        
        return sum / loop;
    }
};
