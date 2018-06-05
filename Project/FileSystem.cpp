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

static string temp_file_dir = base_dir + "Temp Files/";
static unordered_map<string, size_t> read_time_infos, contention_read_infos;

class FileSystem {
public:
    static void measure_all() {
        create_files();

//        vector<int> file_sizes_cache;
//        for (int i = 8; i <= 11; i++) {
//            file_sizes_cache.push_back(pow(2, i));
//        }
//        Measurer::measure_multi(file_read_cache, file_sizes_cache, "File Read Cache", "File Size (MB)", "Time");
//
        vector<int> file_sizes;
        for (int i = 2; i <= 8; i++) {
            file_sizes.push_back(pow(2, i));
        }
        Measurer::measure_multi(seq_file_read_time, file_sizes, "Sequential File Read", "File Size (MB)", "Time");
        Measurer::measure_multi(random_file_read_time, file_sizes, "Random File Read", "File Size (MB)", "Time");
//
//        vector<int> process_nums;
//        for (int i = 1; i <= 9; i++) {
//            process_nums.push_back(i);
//        }
//        Measurer::measure_multi(contention_read_time, process_nums, "Contention Read", "Process", "Time");
//        remove_files();
    }
    
private:
    static string read_time_file_name(int size_MB) {
        return temp_file_dir + "File_" + to_string(size_MB) + "MB";
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
        cout << "Creating temp files" << endl;
        mkdir_if_not_exists(temp_file_dir);
        for (int i = 2; i <= 11; i++) {
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
        cout << "Done!" << endl;
    }
    
    static void remove_files() {
        cout << "Removing temp files" << endl;
        for (auto info : read_time_infos) {
            remove(info.first.data());
        }
        for (auto info : contention_read_infos) {
            remove(info.first.data());
        }
        rmdir_if_exists(temp_file_dir);
        cout << "Done!" << endl;
    }
    
    // Returns read speed in MB/s
    static double read_file_time(string file_name, size_t file_size, size_t step_size, bool no_cache = true, bool random = false) {
        uint64_t start, end;
        
        void *buffer = malloc(step_size);
        int fd = open(file_name.data(), O_SYNC);
        if (fcntl(fd, F_NOCACHE, no_cache) == -1) {
            cout << "Setting F_NOCACHE failed" << endl;
        }
        start = rdtsc();
        if (!random) {
            while (true) {
                ssize_t bytes = read(fd, buffer, step_size);
                if (bytes <= 0) {
                    break;
                }
            }
        } else {
            int repeat = (int)file_size / step_size;
            for (int i = 0; i < repeat; i++) {
                lseek(fd, (arc4random() % repeat) * step_size, SEEK_SET);
                read(fd, buffer, step_size);
            }
        }
        end = rdtsc();
        free(buffer);
        close(fd);
        
        return (file_size / pow(2, 20)) / ((end - start) / pow(10, 9));
    }
    
    static double file_read_cache(int size_MB) {
        string file_name = read_time_file_name(size_MB);
        size_t file_size = read_time_infos[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        
        return read_file_time(file_name, file_size, step_size, false);
    }
    
    static double seq_file_read_time(int size_MB) {
        string file_name = read_time_file_name(size_MB);
        size_t file_size = read_time_infos[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        double MB_per_second = read_file_time(file_name, file_size, step_size);
        return MB_per_second;
    }
    
    static double random_file_read_time(int size_MB) {
        string file_name = read_time_file_name(size_MB);
        size_t file_size = read_time_infos[file_name];
        size_t step_size = pow(2, 12); // 4KB
        double MB_per_second = read_file_time(file_name, file_size, step_size, true, true);
        return MB_per_second;
    }
    
    static double contention_read_time(int process_num) {
        system("sudo -S purge");
        
        for (int block_no = 0; block_no < process_num; block_no++) {
            int pid;
            if ((pid = fork()) < 0) {
                perror("fork");
                abort();
            } else if (pid == 0) {
                string file_name = contention_read_file_name(block_no + 1);
                size_t file_size = contention_read_infos[file_name];
                size_t step_size = min(file_size, (size_t)pow(2, 30));
                read_file_time(file_name, file_size, step_size);
                exit(EXIT_SUCCESS);
            }
        }
        
        string file_name = contention_read_file_name(0);
        size_t file_size = contention_read_infos[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        return read_file_time(file_name, file_size, step_size);
    }
};
