//
//  CPU.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "Measurer.hpp"
#include <pthread.h>

static int CPU_loop = 10;
static int parent_pipe[2], child_pipe[2];

class CPU {
public:
    static void measure_all() {
        Measurer::measure(time_overhead, "Time", "Overhead");
        Measurer::measure(loop_overhead, "Loop", "Overhead");
        for (int i = 0; i <= 7; ++i) {
            function<double()> func = [&i](){return procedure_call_overhead(i);};
            Measurer::measure(func, "Procedure Call (" + to_string(i) + " params)", "Overhead");
        }
        Measurer::measure(system_call_overhead, "System Call", "Overhead");
        Measurer::measure(process_create_overhead, "Process Create", "Overhead", Measurer::STD | Measurer::MIN);
        Measurer::measure(thread_create_overhead, "Thread Create", "Overhead", Measurer::STD | Measurer::MIN);
        Measurer::measure(pipe_overhead, "Pipe", "Overhead");
        Measurer::measure(process_context_switch_overhead, "Process Context Switch", "Overhead", Measurer::STD | Measurer::MIN);
        Measurer::measure(thread_context_switch_overhead, "Thread Context Switch", "Overhead", Measurer::STD | Measurer::MIN);
    }
    
private:
    static double time_overhead() {
        uint64_t start, end;
        double sum = 0;
        
        for (int i = 0; i < CPU_loop; ++i) {
            start = rdtsc();
            end = rdtsc();
            sum += (end - start);
        }
        
        return sum / CPU_loop;
    }
    
    static double loop_overhead() {
        uint64_t start, end;
        double sum = 0;
        int loopTime = 10000;
        
        for (int i = 0; i < CPU_loop; ++i) {
            start = rdtsc();
            for (int j = 0; j < loopTime; ++j) {
                // Do nothing
            }
            end = rdtsc();
            sum += (double)(end - start) / loopTime;
        }
        
        return sum / CPU_loop;
    }
    
    static double procedure_call_overhead(int para_num) {
        uint64_t start, end;
        
        auto procedure_call_0 = [](){};
        auto procedure_call_1 = [](int a){};
        auto procedure_call_2 = [](int a, int b){};
        auto procedure_call_3 = [](int a, int b, int c){};
        auto procedure_call_4 = [](int a, int b, int c, int d){};
        auto procedure_call_5 = [](int a, int b, int c, int d, int e){};
        auto procedure_call_6 = [](int a, int b, int c, int d, int e, int f){};
        auto procedure_call_7 = [](int a, int b, int c, int d, int e, int f, int g){};
        
        start = rdtsc();
        switch (para_num) {
            case 1:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_1(0);
                }
                break;
            case 2:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_2(0, 0);
                }
                break;
            case 3:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_3(0, 0, 0);
                }
                break;
            case 4:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_4(0, 0, 0, 0);
                }
                break;
            case 5:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_5(0, 0, 0, 0, 0);
                }
                break;
            case 6:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_6(0, 0, 0, 0, 0, 0);
                }
                break;
            case 7:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_7(0, 0, 0, 0, 0, 0, 0);
                }
                break;
            default:
                for (int i = 0; i < CPU_loop; ++i) {
                    procedure_call_0();
                }
                break;
        }
        end = rdtsc();
        
        return (double)(end - start) / CPU_loop;
    }
    
    static double system_call_overhead() {
        uint64_t start, end;
        
        start = rdtsc();
        for (int i = 0; i < CPU_loop; ++i) {
            chrono::system_clock::now();
        }
        end = rdtsc();
        
        return (double)(end - start) / CPU_loop;
    }
    
    static double process_create_overhead() {
        uint64_t start, end;

        start = rdtsc();
        for (int i = 0; i < CPU_loop; ++i) {
            if (fork() == 0) { // Child
                exit(EXIT_SUCCESS);
            } else { // Parent
                wait(NULL);
            }
        }
        end = rdtsc();
        return (double)(end - start) / CPU_loop;
    }
    
    static double thread_create_overhead() {
        uint64_t start, end;
        pthread_t thread;
        auto start_routine = [](void *)->void *{
            pthread_exit(NULL);
        };
        
        start = rdtsc();
        for (int i = 0; i < CPU_loop; ++i) {
            pthread_create(&thread, NULL, start_routine, NULL);
            pthread_join(thread, NULL);
        }
        end = rdtsc();
        
        return (double)(end - start) / CPU_loop;
    }
    
    static double pipe_overhead() {
        uint64_t start, end;
        
        int pipefd[2];
        pipe(pipefd);
        int data;
        start = rdtsc();
        for (int i = 0; i < CPU_loop; ++i) {
            write(pipefd[1], (void *)&i, sizeof(int));
            read(pipefd[0], (void *)&data, sizeof(int));
        }
        end = rdtsc();
        close(pipefd[0]);
        close(pipefd[1]);
        
        return (double)(end - start) / (2 * CPU_loop);
    }
    
    static double process_context_switch_overhead() {
        uint64_t start, end;
        uint64_t sum = 0;
        
        pipe(parent_pipe);
        pipe(child_pipe);
        int temp = 0;
        for (int i = 0; i < CPU_loop; ++i) {
            if (fork() == 0) { // Child
                end = rdtsc();
                write(parent_pipe[1], (void *)&end, sizeof(uint64_t));
                // Force context switch
                read(child_pipe[0], (void *)&temp, sizeof(int));
                exit(EXIT_SUCCESS);
            } else { // Parent
                start = rdtsc();
                // Force context switch
                read(parent_pipe[0], (void *)&end, sizeof(uint64_t));
                write(child_pipe[1], (void *)&temp, sizeof(int));
                wait(NULL);
                sum += abs((int64_t)(end - start));
            }
        }
        close(parent_pipe[0]);
        close(parent_pipe[1]);
        close(child_pipe[0]);
        close(child_pipe[1]);
        
        return (double)sum / CPU_loop;
    }
    
    static double thread_context_switch_overhead() {
        uint64_t start, end;
        uint64_t sum = 0;
        
        pipe(parent_pipe);
        pipe(child_pipe);
        int temp = 0;
        pthread_t thread;
        auto child_thread = [](void *)->void *{
            uint64_t end = rdtsc();
            int temp = 0;
            write(parent_pipe[1], (void *)&end, sizeof(uint64_t));
            // Force context switch
            read(child_pipe[0], (void *)&temp, sizeof(int));
            pthread_exit(NULL);
        };
        for (int i = 0; i < CPU_loop; ++i) {
            pthread_create(&thread, NULL, child_thread, NULL);
            start = rdtsc();
            // Force context switch
            read(parent_pipe[0], (void *)&end, sizeof(uint64_t));
            write(child_pipe[1], (void *)&temp, sizeof(int));
            pthread_join(thread, NULL);
            sum += abs((int64_t)(end - start));
        }
        close(parent_pipe[0]);
        close(parent_pipe[1]);
        close(child_pipe[0]);
        close(child_pipe[1]);
        
        return (double)sum / CPU_loop;
    }
};
