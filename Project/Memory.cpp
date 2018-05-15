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
        vector<int> sizes;
        for (int i = 12; i < 28; i++) {
            sizes.push_back(pow(2, i));
        }
        Measurer::measure_multi(memory_access_latency, sizes, "Memory Access", "Array Size", "Latency");
        Measurer::measure(memory_read_bandwidth, "Memory Read", "Bandwidth");
        Measurer::measure(memory_write_bandwidth, "Memory Write", "Bandwidth");
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

        size_t total_size = 128 * pow(2, 20), read_size = 4 * pow(2, 10);
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
        
        size_t total_size = 128 * pow(2, 20);
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
        
        size_t file_size = 5 * pow(2, 30), stride = 1024 * pow(2, 20);
        int repeat = (int)(file_size / stride);
        string file_name = base_dir + "temp";
        FILE *fptr = fopen(file_name.data(), "w");
        char *content = (char *)malloc(file_size);
        memset(content, 0, file_size);
        fwrite(content, sizeof(char), file_size, fptr);
        free(content);
        fclose(fptr);
        
        int file_des = open(file_name.data(), O_RDWR);
        char *map = (char *)mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_des, 0);
        
        start = rdtsc();
        char temp;
        for (int i = 0; i < repeat; i++) {
            temp = map[i * stride];
        }
        end = rdtsc();
        
        munmap(map, file_size);
        close(file_des);
        remove(file_name.data());
        
        return (double)(end - start) / repeat;
    }
};
