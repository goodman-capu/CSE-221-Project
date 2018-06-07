//
//  File_System.cpp
//  Project
//
//  Created by Zhanyuan  Yu on 5/29/18.
//  Copyright © 2018 范志康. All rights reserved.
//


#include "Measurer.hpp"
#include <unordered_map>
#include <sys/stat.h>
#include <fcntl.h>

static unordered_map<string, size_t> cache_info, local_read_info, remote_read_info, contention_info;
static int process_max_num;
static size_t block_size = 4 * pow(2, 10); // 4KB

class FileSystem {
public:
    static void measure_all() {
        create_files();
        
        vector<int> cache_sizes;
        for (auto kv : cache_info) {
            cache_sizes.push_back((int)kv.second / pow(2, 20));
        }
        sort(cache_sizes.begin(), cache_sizes.end());
        Measurer::measure_multi(file_read_cache, cache_sizes, "File Read Cache", "File Size (MB)", "Time");

        vector<int> local_file_sizes;
        for (auto kv : local_read_info) {
            local_file_sizes.push_back((int)kv.second / pow(2, 20));
        }
        sort(local_file_sizes.begin(), local_file_sizes.end());
        Measurer::measure_multi(local_seq_read_time, local_file_sizes, "Sequential File Read", "File Size (MB)", "Time");
        Measurer::measure_multi(local_random_read_time, local_file_sizes, "Random File Read", "File Size (MB)", "Time");
        
        vector<int> remote_file_sizes;
        for (auto kv : remote_read_info) {
            remote_file_sizes.push_back((int)kv.second / pow(2, 20));
        }
        sort(remote_file_sizes.begin(), remote_file_sizes.end());
        Measurer::measure_multi(remote_sql_read_time, remote_file_sizes, "Sequential Remote File Read", "File Size (MB)", "Time");
        Measurer::measure_multi(remote_random_read_time, remote_file_sizes, "Random Remote File Read", "File Size (MB)", "Time");
        
        vector<int> process_nums;
        for (int i = 1; i < process_max_num; i++) {
            process_nums.push_back(i);
        }
        Measurer::measure_multi(contention_read_time, process_nums, "Contention Read", "Process", "Time");
        remove_files();
    }
    
private:
    static string temp_file_dir(string base_dir) {
        return base_dir + "Temp Files/";
    }
    
    static string read_file_name(string base_dir, int size_MB) {
        return temp_file_dir(base_dir) + "File_" + to_string(size_MB) + "MB";
    }
    
    static string contention_file_name(string base_dir, int block_no) {
        return temp_file_dir(base_dir) + "Block_" + to_string(block_no);
    }
    
    static void create_file(string file_name, size_t file_size) {
        struct stat file_stat;
        stat(file_name.data(), &file_stat);
        if (S_ISREG(file_stat.st_mode) && file_stat.st_size == file_size) {
            return;
        }
        FILE *fptr = fopen(file_name.data(), "w");
        if (fptr == nullptr) {
            perror(("Creating " + file_name + " failed").data());
            return;
        }
        char *content = (char *)malloc(file_size);
        memset(content, '0', file_size);
        fwrite(content, sizeof(char), file_size, fptr);
        free(content);
        fclose(fptr);
    }
    
    static void create_files() {
        cout << "Creating temp files" << endl;
        mkdir_if_not_exists(temp_file_dir(base_dir));
        mkdir_if_not_exists(temp_file_dir(NFS_base_dir));
        for (int i = 6; i <= 12; i++) {
            string local_cache_name = read_file_name(base_dir, i * pow(2, 10));
            size_t file_size = i * pow(2, 30);
            cache_info[local_cache_name] = file_size;
            create_file(local_cache_name, file_size);
        }
        for (int i = 2; i <= 8; i++) {
            string local_file_name = read_file_name(base_dir, pow(2, i));
            size_t file_size = pow(2, 20 + i);
            local_read_info[local_file_name] = file_size;
            create_file(local_file_name, file_size);
        }
        for (int i = 2; i <= 8; i++) {
            string remote_file_name = read_file_name(NFS_base_dir, pow(2, i));
            size_t file_size = pow(2, 20 + i);
            remote_read_info[remote_file_name] = file_size;
            create_file(remote_file_name, file_size);
        }
        process_max_num = 10;
        for (int i = 0; i < process_max_num; i++) {
            string local_file_name = contention_file_name(base_dir, i);
            size_t file_size = pow(2, 26);
            contention_info[local_file_name] = file_size;
            create_file(local_file_name, file_size);
        }
        cout << "Done!" << endl;
    }
    
    static void remove_files() {
        cout << "Removing temp files" << endl;
        for (auto info : cache_info) {
            remove(info.first.data());
        }
        for (auto info : local_read_info) {
            remove(info.first.data());
        }
        for (auto info : remote_read_info) {
            remove(info.first.data());
        }
        for (auto info : contention_info) {
            remove(info.first.data());
        }
        rmdir_if_exists(temp_file_dir(base_dir));
        rmdir_if_exists(temp_file_dir(NFS_base_dir));
        cout << "Done!" << endl;
    }
    
    // Returns read speed in MB/s
    static double read_file_time(string file_name, size_t file_size, size_t read_size, bool no_cache = true, bool random = false) {
        uint64_t start, end;
        
        void *buffer = malloc(read_size);
        int fd = open(file_name.data(), O_SYNC);
        if (fd == -1) {
            perror("Open file failed");
            return 0;
        }
        if (fcntl(fd, F_NOCACHE, no_cache) == -1) {
            perror("Set F_NOCACHE failed");
            return 0;
        }
        start = rdtsc();
        if (!random) {
            while (true) {
                ssize_t bytes = read(fd, buffer, read_size);
                if (bytes <= 0) {
                    break;
                }
            }
        } else {
            int repeat = (int)file_size / read_size;
            for (int i = 0; i < repeat; i++) {
                lseek(fd, (arc4random() % repeat) * read_size, SEEK_SET);
                read(fd, buffer, read_size);
            }
        }
        end = rdtsc();
        free(buffer);
        close(fd);
        
        return (file_size / pow(2, 20)) / ((end - start) / pow(10, 9));
    }
    
    static inline double ms_per_block(double MB_per_second) {
        return 1000 / (MB_per_second * (pow(2, 20) / block_size));
    }
    
    static double file_read_cache(int size_MB) {
        string file_name = read_file_name(base_dir, size_MB);
        size_t file_size = local_read_info[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        return read_file_time(file_name, file_size, step_size, false);
    }
    
    static double local_seq_read_time(int size_MB) {
        string file_name = read_file_name(base_dir, size_MB);
        size_t file_size = local_read_info[file_name];
        return ms_per_block(read_file_time(file_name, file_size, block_size));
    }
    
    static double local_random_read_time(int size_MB) {
        string file_name = read_file_name(base_dir, size_MB);
        size_t file_size = local_read_info[file_name];
        return ms_per_block(read_file_time(file_name, file_size, block_size, true, true));
    }
    
    static double remote_sql_read_time(int size_MB) {
        string file_name = read_file_name(NFS_base_dir, size_MB);
        size_t file_size = remote_read_info[file_name];
        return ms_per_block(read_file_time(file_name, file_size, block_size));
    }
    
    static double remote_random_read_time(int size_MB) {
        string file_name = read_file_name(NFS_base_dir, size_MB);
        size_t file_size = remote_read_info[file_name];
        return ms_per_block(read_file_time(file_name, file_size, block_size, true, true));
    }
    
    static double contention_read(int block_no) {
        string file_name = contention_file_name(base_dir, block_no);
        size_t file_size = contention_info[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        return ms_per_block(read_file_time(file_name, file_size, step_size));
    }
    
    static double contention_read_time(int process_num) {
        system("sudo -S purge");
        
        for (int i = 1; i <= process_num; i++) {
            int pid;
            if ((pid = fork()) < 0) {
                perror("fork() failed");
                abort();
            } else if (pid == 0) {
                contention_read(i);
                exit(EXIT_SUCCESS);
            }
        }
        
        return contention_read(0);
    }
};
