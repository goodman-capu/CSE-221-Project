//
//  Memory.cpp
//  Project
//
//  Created by 范志康 on 2018/5/12.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "Measurer.hpp"
#include <fcntl.h>
#include <sys/mman.h>

static int mem_loop = 1e6;

class Memory {
public:
    static void measure_all() {
//        vector<int> sizes;
//        for (int i = 12; i < 26; i++) {
//            sizes.push_back(pow(2, i));
//        }
//        Measurer::measure_multi(memory_access_latency, sizes, "Memory Access", "Array Size", "Latency");
//        Measurer::measure(memory_read_bandwidth, "Memory Read", "Bandwidth");
//        Measurer::measure(memory_write_bandwidth, "Memory Write", "Bandwidth");
        Measurer::measure(page_fault_time, "Page Fault", "Time");
    }
    
private:
    static double memory_access_latency(int size) {
        uint64_t start, end;
        int stride = size / 1024;
        
        int *array = (int *)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            array[i] = (i + stride) % size;
        }
        start = rdtsc();
        int temp = 0;
        for (int i = 0; i < mem_loop; i++) {
            temp = array[temp];
        }
        end = rdtsc();
        free(array);
        return (double)(end - start) / mem_loop;
    }
    
    static double memory_read_bandwidth() {
        uint64_t start, end;

        size_t total_size = 128 * 1024 * 1024, read_size = 4 * 1024;
        long repeat = total_size / read_size;
        char *data = (char *)malloc(total_size);
        char *buffer = (char *)malloc(read_size);
        memset(data, 1, total_size);
        memset(buffer, 0, read_size);
        start = rdtsc();
        for (int i = 0; i < repeat; i++) {
            memcpy(buffer, data + i * read_size, read_size);
        }
        end = rdtsc();
        free(data);
        free(buffer);

        return total_size / (double)(end - start);
    }
    
    static double memory_write_bandwidth() {
        uint64_t start, end;
        
        size_t total_size = 128 * 1024 * 1024;
        char *data = (char *)malloc(total_size);
        memset(data, 0, total_size);
        start = rdtsc();
        memset(data, 1, total_size);
        end = rdtsc();
        free(data);
        
        return total_size / (double)(end - start);
    }
    
    static double page_fault_time() {
        uint64_t start, end;
        
        size_t total_size = (unsigned long)pow(2, 30) * 20, read_size = 4 * 1024;
        long repeat = 5;
        size_t stride = total_size / repeat;
        char *data = (char *)malloc(total_size);
        char *buffer = (char *)malloc(read_size);
        memset(data, 0, total_size);
        memset(buffer, 0, read_size);
        for (int i = 0; i < repeat; i++) {
            memset(data + i * stride, 1, read_size);
        }
        start = rdtsc();
        for (int i = 0; i < repeat; i++) {
            memcpy(buffer, data + i * stride, read_size);
        }
        end = rdtsc();
        free(data);
        free(buffer);
        
        return (double)(end - start) / repeat;
    }
};
