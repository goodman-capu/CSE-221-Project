//
//  CPU.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "Measurer.cpp"

static int loop = 1000;

class CPU {
public:
    static void measureAll() {
        Measurer::measure(timeOverhead, "Time");
        Measurer::measure(loopOverhead, "Loop");
        for (int i = 0; i <= 7; ++i) {
            function<double()> func = [&i](){return procedureCallOverhead(i);};
            Measurer::measure(func, "Procedure Call (" + to_string(i) + " params)");
        }
        Measurer::measure(systemCallOverhead, "System Call");
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
        
        start = __rdtsc();
        switch (paraNum) {
            case 1:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0);
                }
                break;
            case 2:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0);
                }
                break;
            case 3:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0);
                }
                break;
            case 4:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0);
                }
                break;
            case 5:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0, 0);
                }
                break;
            case 6:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0, 0, 0);
                }
                break;
            case 7:
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0, 0, 0, 0);
                }
                break;
            default:
                for (int i = 0; i < loop; ++i) {
                    procedureCall();
                }
                break;
        }
        end = __rdtsc();
        
        return (double)(end - start) / loop;
    }
    
    static double systemCallOverhead() {
        uint64_t start, end;
        
        start = __rdtsc();
        for (int i = 0; i < loop; ++i) {
            chrono::system_clock::now();
        }
        end = __rdtsc();
        
        return (double)(end - start) / loop;
    }
};
