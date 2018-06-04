//
//  File_System.cpp
//  Project
//
//  Created by Zhanyuan  Yu on 5/29/18.
//  Copyright © 2018 范志康. All rights reserved.
//


#include "Measurer.hpp"
#include <fcntl.h>
#include <sys/mman.h>

class FileSystem {
public:
    static void measure_all() {
        create_files(); //generate the files needed for this file system measurement
        vector<int> file_sizes;
        for (int i = 2; i <= 8; i++) {
            file_sizes.push_back(i);
        }
        vector<int> num;
        for (int i = 1; i <= 9; i++) {
            num.push_back(i);
        }
        Measurer::measure_multi(seq_file_read_time, file_sizes, "Sequential File Read", "File Size", "Time");
        Measurer::measure_multi(random_file_read_time, file_sizes, "Random File Read", "File Size", "Time");
        Measurer::measure_multi(contention_read_time, num, "Contention Read", "Process", "Time");
        remove_files();
    }
    
private:
    static string file_name_of_size(size_t size_MB) {
        return base_dir + "File_" + to_string(size_MB) + "MB.data";
    }
    
    static void create_files() {
        for (int i = 2; i <= 8; i++) {
            int pow_size = 20 + i;
            size_t file_size = pow(2, pow_size);
            string file_name = base_dir + "read_time/File_" + to_string((int)pow(2,i)) + "MB.txt";
            FILE *fptr = fopen(file_name.data(), "w");
            char *content = (char *)malloc(file_size);
            memset(content, '0', file_size);
            fwrite(content, sizeof(char), file_size, fptr);
            free(content);
            fclose(fptr);
        }
        for (int i = 0; i < 10; i++) {
            size_t file_size = pow(2, 26);
            string file_name = base_dir + "contention_read/block" + to_string(i) + ".txt";

            FILE *fptr = fopen(file_name.data(), "w");
            char *content = (char *)malloc(file_size);
            memset(content, '0', file_size);
            fwrite(content, sizeof(char), file_size, fptr);
            free(content);
            fclose(fptr);
            cout << file_name << endl;
        }
    }
    
    static void remove_files() {
        for (int i = 2; i <= 8; i++) {
            string file_name = base_dir + "read_time/File_" + to_string((int)pow(2,i)) + "MB.txt";
            remove(file_name.data());
        }
        for (int i = 0; i < 10; i++) {
            string file_name = base_dir + "contention_read/block" + to_string(i) + ".txt";
            remove(file_name.data());
        }
    }
    
    static double seq_file_read_time(int i) {
        uint64_t start, end, total_time = 0;
        string file_name = base_dir + "read_time/File_" + to_string((int)pow(2,i)) + "MB.txt";
        size_t total_size = pow(2, 20 + i), read_size =  pow(2, 12);
        void *buffer = malloc(read_size);
        
        int fd = open(file_name.data(), O_SYNC);
        if(fcntl(fd, F_NOCACHE, 1) == -1) {
            cout << "fail" << endl;
        }
        while (TRUE) {
            start = rdtsc();
            ssize_t bytes = read(fd, buffer, read_size);
            if (bytes <= 0) {
                break;
            }
            end = rdtsc();
            total_time += end - start;
        }
        free(buffer);
        close(fd);
        return total_time / (double)(total_size / read_size);
    }
    
    static double random_file_read_time(int i) {
        uint64_t start, end, total_time = 0;
        string file_name = base_dir + "read_time/File_" + to_string((int)pow(2,i)) + "MB.txt";
        size_t total_size = pow(2, 20 + i), read_size =  pow(2, 12);
        void * buffer = malloc(read_size);
        
        int fd = open(file_name.data(), O_SYNC);
        if(fcntl(fd, F_NOCACHE, 1) == -1) {
            cout << "fail" << endl;
        }
        off_t num = total_size / read_size;
        for (int i = 0; i < num; i++) {
            off_t j = arc4random() % num;
            lseek(fd, j * read_size, SEEK_SET);
            start = rdtsc();
            read(fd, buffer, read_size);
            end = rdtsc();
            total_time += end - start;
        }
        close(fd);
        free(buffer);
        return total_time / num;
    }
    
    static double read_file(int i) {
        uint64_t start, end, total_time = 0;
        string file_name = base_dir + "contention_read/block" + to_string(i) + ".txt";

        size_t total_size = pow(2, 26);
        size_t read_size =  pow(2, 12);
        void *buffer = malloc(read_size);

        int fd = open(file_name.data(), O_SYNC);
        if(fcntl(fd, F_NOCACHE, 1) == -1) {
            cout << "fail" << endl;
        }
        while (TRUE) {
            start = rdtsc();
            ssize_t bytes = read(fd, buffer, read_size);
            if (bytes <= 0) {
                break;
            }
            end = rdtsc();
            total_time += end - start;
        }
        free(buffer);
        close(fd);
        return total_time / (double)(total_size / read_size);
        
    }
    
    static double contention_read_time(int num) {
        pid_t pid[10];
        double read_time[10] = {0.0};
        double parent_read_time = 0.0;
        
        for ( int i = 0; i < num; i++) {
            if ((pid[i] = fork()) < 0) {
                perror("fork");
                abort();
            } else if (pid[i] == 0) {
                read_time[i] = read_file(i + 1);
                exit(0);
            } else{
            }
        }
        
        parent_read_time = read_file(0);
        return parent_read_time;
    }
};



