//
//  File_System.cpp
//  Project
//
//  Created by Zhanyuan  Yu on 5/29/18.
//  Copyright © 2018 范志康. All rights reserved.
//


#include "Measurer.hpp"
#include <unordered_map>
#include <fcntl.h>

static string temp_file_dir = "Temp Files/";
static unordered_map<string, size_t> read_time_infos, contention_read_infos;

class FileSystem {
public:
    static void measure_all() {
        create_files();
        
        vector<int> file_sizes;
        for (int i = 2; i <= 8; i++) {
            file_sizes.push_back(i);
        }
        Measurer::measure_multi(seq_file_read_time, file_sizes, "Sequential File Read", "File Size", "Time");
        Measurer::measure_multi(random_file_read_time, file_sizes, "Random File Read", "File Size", "Time");
        
        vector<int> process_nums;
        for (int i = 1; i <= 9; i++) {
            process_nums.push_back(i);
        }
        Measurer::measure_multi(contention_read_time, process_nums, "Contention Read", "Process", "Time");
        remove_files();
    }
    
private:
    static string read_time_file_name(size_t file_size_MB) {
        return temp_file_dir + "File_" + to_string(file_size_MB) + "MB";
    }
    
    static string contention_read_file_name(int block_no) {
        return temp_file_dir + "Block_" + to_string(block_no);
    }
    
    static void create_file(string file_name, size_t file_size) {
        if (access(file_name.data(), F_OK) == 0) {
            return;
        }
        FILE *fptr = fopen(file_name.data(), "w");
        char *content = (char *)malloc(file_size);
        memset(content, '0', file_size);
        fwrite(content, sizeof(char), file_size, fptr);
        free(content);
        fclose(fptr);
    }
    
    static void create_files() {
        mkdir_if_not_exists(temp_file_dir);
        for (int i = 2; i <= 8; i++) {
            string file_name = read_time_file_name(pow(2, i));
            size_t file_size = pow(2, 20 + i);
            read_time_infos[file_name] = file_size;
            create_file(file_name, file_size);
        }
        for (int i = 0; i < 10; i++) {
            string file_name = contention_read_file_name(i);
            size_t file_size = pow(2, 26);
            contention_read_infos[file_name] = file_size;
            create_file(file_name, file_size);
        }
    }
    
    static void remove_files() {
        for (auto info : read_time_infos) {
            remove(info.first.data());
        }
        for (auto info : contention_read_infos) {
            remove(info.first.data());
        }
        rmdir_if_exists(temp_file_dir);
    }
    
    static double seq_file_read_time(int i) {
        uint64_t start, end, total_time = 0;
        
        string file_name = read_time_file_name(pow(2, i));
        size_t total_size = read_time_infos[file_name];
        size_t read_size = pow(2, 12);
        void *buffer = malloc(read_size);
        
        int fd = open(file_name.data(), O_SYNC);
        if (fcntl(fd, F_NOCACHE, 1) == -1) {
            cout << "fail" << endl;
        }
        while (true) {
            start = rdtsc();
            ssize_t bytes = read(fd, buffer, read_size);
            if (bytes <= 0) {
                break;
            }
            end = rdtsc();
            total_time += (end - start);
        }
        close(fd);
        free(buffer);
        
        return total_time / (double)(total_size / read_size);
    }
    
    static double random_file_read_time(int i) {
        uint64_t start, end, total_time = 0;
        
        string file_name = read_time_file_name(pow(2, i));
        size_t total_size = read_time_infos[file_name];
        size_t read_size = pow(2, 12);
        void *buffer = malloc(read_size);
        
        int fd = open(file_name.data(), O_SYNC);
        if (fcntl(fd, F_NOCACHE, 1) == -1) {
            cout << "fail" << endl;
        }
        off_t num = total_size / read_size;
        for (int i = 0; i < num; i++) {
            off_t j = arc4random() % num;
            lseek(fd, j * read_size, SEEK_SET);
            start = rdtsc();
            read(fd, buffer, read_size);
            end = rdtsc();
            total_time += (end - start);
        }
        free(buffer);
        close(fd);
        
        return total_time / num;
    }
    
    static double read_file(int block_no) {
        uint64_t start, end, total_time = 0;
        
        string file_name = contention_read_file_name(block_no);
        size_t total_size = contention_read_infos[file_name];
        size_t read_size = pow(2, 12);
        void *buffer = malloc(read_size);

        int fd = open(file_name.data(), O_SYNC);
        if (fcntl(fd, F_NOCACHE, 1) == -1) {
            cout << "fail" << endl;
        }
        while (true) {
            start = rdtsc();
            ssize_t bytes = read(fd, buffer, read_size);
            if (bytes <= 0) {
                break;
            }
            end = rdtsc();
            total_time += (end - start);
        }
        free(buffer);
        close(fd);
        
        return total_time / (double)(total_size / read_size);
    }
    
    static double contention_read_time(int process_num) {
        system("sudo -S purge");
        
        int pid;
        for (int i = 0; i < process_num; i++) {
            if ((pid = fork()) < 0) {
                perror("fork");
                abort();
            } else if (pid == 0) {
                read_file(i + 1);
                exit(EXIT_SUCCESS);
            }
        }
        
        return read_file(0);
    }
};
