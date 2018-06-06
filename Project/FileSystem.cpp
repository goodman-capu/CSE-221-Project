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

static unordered_map<string, size_t> local_read_info, local_contention_info, remote_read_info;
static size_t block_size = pow(2, 12); // 4KB

class FileSystem {
public:
    static void measure_all() {
        create_files();
        
        vector<int> file_sizes_cache;
        for (int i = 5; i <= 14; i++) {
            file_sizes_cache.push_back(pow(2, i));
        }
        Measurer::measure_multi(file_read_cache, file_sizes_cache, "File Read Cache", "File Size (MB)", "Time");

        vector<int> file_sizes;
        for (int i = 2; i <= 8; i++) {
            file_sizes.push_back(pow(2, i));
        }
        Measurer::measure_multi(local_seq_read_time, file_sizes, "Sequential File Read", "File Size (MB)", "Time");
        Measurer::measure_multi(local_random_read_time, file_sizes, "Random File Read", "File Size (MB)", "Time");
        Measurer::measure_multi(remote_sql_read_time, file_sizes, "Sequential Remote File Read", "File Size (MB)", "Time");
        Measurer::measure_multi(remote_random_read_time, file_sizes, "Random Remote File Read", "File Size (MB)", "Time");

        vector<int> process_nums;
        for (int i = 1; i <= 9; i++) {
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
        for (int i = 2; i <= 14; i++) {
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
        for (int i = 0; i < 10; i++) {
            string local_file_name = contention_file_name(base_dir, i);
            size_t file_size = pow(2, 26);
            local_contention_info[local_file_name] = file_size;
            create_file(local_file_name, file_size);
        }
        cout << "Done!" << endl;
    }
    
    static void remove_files() {
        cout << "Removing temp files" << endl;
        for (auto info : local_read_info) {
            remove(info.first.data());
        }
        for (auto info : remote_read_info) {
            remove(info.first.data());
        }
        for (auto info : local_contention_info) {
            remove(info.first.data());
        }
        rmdir_if_exists(temp_file_dir(base_dir));
        rmdir_if_exists(temp_file_dir(NFS_base_dir));
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
        string file_name = read_file_name(base_dir, size_MB);
        size_t file_size = local_read_info[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        
        return read_file_time(file_name, file_size, step_size, false);
    }
    
    static double local_seq_read_time(int size_MB) {
        string file_name = read_file_name(base_dir, size_MB);
        size_t file_size = local_read_info[file_name];
        double MB_per_second = read_file_time(file_name, file_size, block_size);
        return MB_per_second;
    }
    
    static double local_random_read_time(int size_MB) {
        string file_name = read_file_name(base_dir, size_MB);
        size_t file_size = local_read_info[file_name];
        double MB_per_second = read_file_time(file_name, file_size, block_size, true, true);
        return MB_per_second;
    }
    
    static double remote_sql_read_time(int size_MB) {
        string file_name = read_file_name(NFS_base_dir, size_MB);
        size_t file_size = remote_read_info[file_name];
        double MB_per_second = read_file_time(file_name, file_size, block_size);
        return MB_per_second;
    }
    
    static double remote_random_read_time(int size_MB) {
        string file_name = read_file_name(NFS_base_dir, size_MB);
        size_t file_size = remote_read_info[file_name];
        double MB_per_second = read_file_time(file_name, file_size, block_size, true, true);
        return MB_per_second;
    }
    
    static double contention_read(int block_no) {
        string file_name = contention_file_name(base_dir, block_no);
        size_t file_size = local_contention_info[file_name];
        size_t step_size = min(file_size, (size_t)pow(2, 30));
        return read_file_time(file_name, file_size, step_size);
    }
    
    static double contention_read_time(int process_num) {
        system("sudo -S purge");
        
        for (int i = 0; i < process_num; i++) {
            int pid;
            if ((pid = fork()) < 0) {
                perror("fork");
                abort();
            } else if (pid == 0) {
                contention_read(i + 1);
                exit(EXIT_SUCCESS);
            }
        }
        
        return contention_read(0);
    }
};
