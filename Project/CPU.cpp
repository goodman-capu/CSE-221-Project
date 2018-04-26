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
static int parentPipe[2], childPipe[2];

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
        Measurer::measure(pipeOverhead, "Pipe");
        Measurer::measure(processContextSwitchOverhead, "Process Context Switch");
        Measurer::measure(threadContextSwitchOverhead, "Thread Context Switch");
    }
    
private:
    static double timeOverhead() {
        uint64_t start, end;
        double sum = 0;
        
        for (int i = 0; i < loop; ++i) {
            start = rdtsc();
            end = rdtsc();
            sum += (end - start);
        }
        
        return sum / loop;
    }
    
    static double loopOverhead() {
        uint64_t start, end;
        double sum = 0;
        int loopTime = 10000;
        
        for (int i = 0; i < loop; ++i) {
            start = rdtsc();
            for (int j = 0; j < loopTime; ++j) {
                // Do nothing
            }
            end = rdtsc();
            sum += (double)(end - start) / loopTime;
        }
        
        return sum / loop;
    }
    
    static double procedureCallOverhead(int paraNum) {
        uint64_t start, end;
        
        auto procedureCall_0 = [](){};
        auto procedureCall_1 = [](int a){};
        auto procedureCall_2 = [](int a, int b){};
        auto procedureCall_3 = [](int a, int b, int c){};
        auto procedureCall_4 = [](int a, int b, int c, int d){};
        auto procedureCall_5 = [](int a, int b, int c, int d, int e){};
        auto procedureCall_6 = [](int a, int b, int c, int d, int e, int f){};
        auto procedureCall_7 = [](int a, int b, int c, int d, int e, int f, int g){};
        
        start = rdtsc();
        switch (paraNum) {
            case 1:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_1(0);
                }
                break;
            case 2:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_2(0, 0);
                }
                break;
            case 3:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_3(0, 0, 0);
                }
                break;
            case 4:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_4(0, 0, 0, 0);
                }
                break;
            case 5:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_5(0, 0, 0, 0, 0);
                }
                break;
            case 6:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_6(0, 0, 0, 0, 0, 0);
                }
                break;
            case 7:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_7(0, 0, 0, 0, 0, 0, 0);
                }
                break;
            default:
                for (int i = 0; i < loop; ++i) {
                    procedureCall_0();
                }
                break;
        }
        end = rdtsc();
        
        return (double)(end - start) / loop;
    }
    
    static double systemCallOverhead() {
        uint64_t start, end;
        
        start = rdtsc();
        for (int i = 0; i < loop; ++i) {
            chrono::system_clock::now();
        }
        end = rdtsc();
        
        return (double)(end - start) / loop;
    }
    
    static double processCreateOverhead() {
        uint64_t start, end;

        start = rdtsc();
        for (int i = 0; i < loop; ++i) {
            if (fork() == 0) { // Child
                exit(0);
            } else { // Parent
                wait(NULL);
            }
        }
        end = rdtsc();
        return (double)(end - start) / loop;
    }
    
    static double threadCreateOverhead() {
        uint64_t start, end;
        pthread_t thread;
        auto startRoutine = [](void *)->void *{
            pthread_exit(NULL);
        };
        
        start = rdtsc();
        for (int i = 0; i < loop; ++i) {
            pthread_create(&thread, NULL, startRoutine, NULL);
            // pthread_join() function suspend execution of the calling thread until the target thread terminates
            pthread_join(thread, NULL);
        }
        end = rdtsc();
        
        return (double)(end - start) / loop;
    }
    
    static double pipeOverhead() {
        uint64_t start, end;
        
        int pipefd[2];
        pipe(pipefd);
        int data;
        start = rdtsc();
        for (int i = 0; i < loop; ++i) {
            write(pipefd[1], (void *)&i, sizeof(int));
            read(pipefd[0], (void *)&data, sizeof(int));
        }
        end = rdtsc();
        
        return (double)(end - start) / (2 * loop);
    }
    
    static double processContextSwitchOverhead() {
        uint64_t start, end;
        
        pipe(parentPipe);
        pipe(childPipe);
        
        if (fork() == 0) { // Child
            int data;
            for (int i = 0; i < loop; ++i) {
                write(childPipe[1], (void *)&i, sizeof(int));
                read(parentPipe[0], (void *)&data, sizeof(int));
            }
            exit(0);
        } else { // Parent
            int data;
            start = rdtsc();
            for (int i = 0; i < loop; ++i) {
                write(parentPipe[1], (void *)&i, sizeof(int));
                read(childPipe[0], (void *)&data, sizeof(int));
            }
            wait(NULL);
            end = rdtsc();
        }
        
        return (double)(end - start) / loop;
    }
    
    static double threadContextSwitchOverhead() {
        uint64_t start, end;
        
        pipe(parentPipe);
        pipe(childPipe);
        pthread_t thread;
        auto childThread = [](void *)->void *{
            int data;
            for (int i = 0; i < loop; ++i) {
                write(childPipe[1], (void *)&i, sizeof(int));
                read(parentPipe[0], (void *)&data, sizeof(int));
            }
            pthread_exit(NULL);
        };
        pthread_create(&thread, NULL, childThread, NULL);
        int data;
        start = rdtsc();
        for (int i = 0; i < loop; ++i) {
            write(parentPipe[1], (void *)&i, sizeof(int));
            read(childPipe[0], (void *)&data, sizeof(int));
        }
        pthread_join(thread, NULL);
        end = rdtsc();

        return (double)(end - start) / loop;
    }
};
