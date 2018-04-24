//
//  CPU.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "Measurer.cpp"
#include <pthread.h>

static int loop = 1000;
static int pipefd[2];

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
        Measurer::measure(processCreateOverhead, "Process Create");
        Measurer::measure(threadCreateOverhead, "Thread Create");
        Measurer::measure(processContextSwitchOverhead, "Process Context Switch");
        Measurer::measure(threadContextSwitchOverhead, "Thread Context Switch");
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
    
    static double processCreateOverhead() {
        uint64_t start, end;
        pid_t pid;
        start = __rdtsc();
        for (int i = 0; i < loop; ++i) {
            pid = fork();
            if (pid == 0) {
                exit(0);
            } else if (pid < 0) {
                exit(1);
            } else {
                wait(NULL);
            }
        }
        end = __rdtsc();
        return (double)(end - start) / loop;
    }
    
    static double threadCreateOverhead() {
        uint64_t start, end;
        pthread_t thread;
        auto startRoutine = [](void *)->void *{pthread_exit(NULL);};
        
        start = __rdtsc();
        for (int i = 0; i < loop; ++i) {
            pthread_create(&thread, NULL, startRoutine, NULL);
            // pthread_join() function suspend execution of the calling thread until the target thread terminates
            pthread_join(thread, NULL);
        }
        end = __rdtsc();
        
        return (double)(end - start) / loop;
    }
    
    // https://linux.die.net/man/2/pipe
    static double processContextSwitchOverhead() {
        uint64_t start, end;
        pid_t pid;
        uint64_t sum = 0;
        int num = 0;
        pipe(pipefd);
        for (int i = 0; i < loop; ++i) {
            pid = fork();
            if (pid != 0) {
                start = __rdtsc();
                
                wait(NULL);
                read(pipefd[0], (void *)&end, sizeof(uint64_t));
            } else {
                end = __rdtsc();
                
                write(pipefd[1], (void *)&end, sizeof(uint64_t));
                exit(0);
            }
            if (end > start) {
                num ++;
                sum += end - start;
            }
        }
        
        return (double)(sum) / num;
    }
    
    static void *sendEnd(void *) {
        uint64_t end = __rdtsc();
        
        write(pipefd[1], (void*)&end, sizeof(uint64_t));
        
        pthread_exit(NULL);
    }
    
    static double threadContextSwitchOverhead() {
        uint64_t start, end;
        pthread_t thread;
        uint64_t sum = 0;
        int num = 0;
        pipe(pipefd);
        for (int i = 0; i < loop; ++i) {
            pthread_create(&thread, NULL, sendEnd, NULL);
            start = __rdtsc();
            pthread_join(thread, NULL);
            read(pipefd[0], (void*)&end, sizeof(uint64_t));
            if (end > start) {
                num ++;
                sum += end - start;
            }
        }
        return (double)(sum) / num;
    }
};
