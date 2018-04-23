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
//        Measurer::measure(timeOverhead, "Time");
//        Measurer::measure(loopOverhead, "Loop");
//        for (int i = 0; i <= 7; ++i) {
//            function<double()> func = [&i](){return procedureCallOverhead(i);};
//            Measurer::measure(func, "Procedure Call (" + to_string(i) + " params)");
//        }
//        Measurer::measure(systemCallOverhead, "System Call");
//        Measurer::measure(processCreateOverhead, "processCreate");
        Measurer::measure(processContextSwitchOverhead, "processContextSwitch");
       // Measurer::measure(threadCreateOverhead, "threadCreate");
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
        
        switch (paraNum) {
            case 1:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0);
                }
                end = __rdtsc();
                break;
            case 2:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0);
                }
                end = __rdtsc();
                break;
            case 3:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0);
                }
                end = __rdtsc();
                break;
            case 4:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0);
                }
                end = __rdtsc();
                break;
            case 5:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0, 0);
                }
                end = __rdtsc();
                break;
            case 6:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0, 0, 0);
                }
                end = __rdtsc();
                break;
            case 7:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall(0, 0, 0, 0, 0, 0, 0);
                }
                end = __rdtsc();
                break;
            default:
                start = __rdtsc();
                for (int i = 0; i < loop; ++i) {
                    procedureCall();
                }
                end = __rdtsc();
                break;
        }
        
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
        //int status;
        uint64_t sum = 0;
        for (int i = 0; i < loop; ++i) {
            start = __rdtsc();
            pid = fork();
            if (pid == 0) {
                exit(0);
            }
            else if(pid < 0)
                exit(1);
            else {
                wait(NULL);
                //waitpid(pid,&status,0);
                end = __rdtsc();
            }
            sum += end - start;
        }
        //end = __rdtsc();
        
        return (double)(sum) / loop;
    }
    static void* startRoutine(void *) {
        pthread_exit(NULL);
    }
    static double threadCreateOverhead() {
        uint64_t start, end;
        pthread_t thread;
        start = __rdtsc();
        for (int i = 0; i < loop; ++i) {
            pthread_create(&thread, NULL, startRoutine, NULL);
            pthread_join(thread, NULL);
        }
        end = __rdtsc();

        return (double)(end - start) / loop;
    }
    
    //https://linux.die.net/man/2/pipe
    static int pipefd[2];
    static double processContextSwitchOverhead() {
        uint64_t start, end;
        pid_t pid;
        uint64_t sum = 0;
        int num = 0;
        pipe(pipefd);
        start = __rdtsc();
        pthread_t thread;
        for (int i = 0; i < loop; ++i) {
            pthread_create(&thread, NULL, startRoutine, NULL);
            pthread_join(thread, NULL);
        }
        end = __rdtsc();
        for (int i = 0; i < loop; ++i) {
            start = __rdtsc();
            pid = fork();
            
            if(pid != 0)
            {
                start = __rdtsc();
                
                wait(NULL);
                read(pipefd[0], (void*)&end, sizeof(uint64_t));
            }
            else {
                end = __rdtsc();
                
                write(pipefd[1], (void*)&end, sizeof(uint64_t));
                exit(0);

            }
            if(end > start)
            {
                num ++;
                sum += end - start;
            }
        }
        //end = __rdtsc();
        
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
        int pipefd[2];
        pipe(pipefd);
        for (int i = 0; i < loop; ++i) {
            start = __rdtsc();
            pthread_create(&thread, NULL, sendEnd, NULL);
            pthread_join(thread, NULL);
            read(pipefd[0], (void*)&end, sizeof(uint64_t));
        }
        if(end > start)
        {
            num ++;
            sum += end - start;
        }
        return (double)(sum) / num;
    }
};
