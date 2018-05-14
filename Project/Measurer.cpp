//
//  Measurer.cpp
//  Project
//
//  Created by 范志康 on 2018/5/12.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include "Measurer.hpp"
#include <sys/stat.h>

m_stat get_stats(vector<double> &results) {
    m_stat s;
    s.mean = accumulate(results.begin(), results.end(), 0.0) / results.size();
    double var = 0.0;
    for_each(results.begin(), results.end(), [&](const double val) {
        var += pow((val - s.mean), 2);
    });
    s.std = sqrt(var / (results.size() - 1));
    s.min = *min_element(results.begin(), results.end());
    return s;
}

void output_result(ostream &os, int counter, double result) {
    os << left << setw(5) << to_string(counter) + ": ";
    os << left << setw(12) << result;
    if (counter % 5 == 0) {
        os << endl;
    }
}

void output_multi_result(ostream &os, vector<pair<int, m_stat>> &stats, string param_name) {
    os << param_name << "\tMean\tSTD" << endl;
    for (auto stat : stats) {
        os << stat.first << "\t" << stat.second.mean << "\t" << stat.second.std << endl;
    }
}

void Measurer::measure(function<double()> func, string name, string type, int filters) {
    cout << "Measure " << name << " " << type << " Begins" << endl;
    
    string file_name = base_dir + name + ".txt";
    _measure(func, cout, file_name, filters);
    
    cout << "Measure " << name << " " << type << " Completes" << endl;
}

void Measurer::measure_multi(function<double(int)> func, vector<int> params, string func_name, string param_name, string type, int filters) {
    cout << "Measure " << func_name << " Begins" << endl;
    
    vector<pair<int, m_stat>> stats;
    for (int param : params) {
        function<double()> bind_func = [&param, &func](){return func(param);};
        string bind_name = func_name + " (" + param_name + " " + to_string(param) + ")";
        cout << "Measure " << bind_name << " " << type << " Begins" << endl;
        
        string dir_name = base_dir + func_name + "/";
        struct stat dir_stat;
        stat(dir_name.data(), &dir_stat);
        if (!S_ISDIR(dir_stat.st_mode)) {
            mkdir(dir_name.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        
        string file_name = dir_name + param_name + " " + to_string(param) + ".txt";
        m_stat s = _measure(bind_func, cout, file_name, filters);
        stats.push_back({param, s});
        
        cout << "Measure " << bind_name << " " << type << " Completes" << endl;
    }
    
    output_multi_result(cout, stats, param_name);
    fstream file;
    file.open(base_dir + func_name + ".txt", ios::out);
    output_multi_result(file, stats, param_name);
    file.close();
    
    cout << "Measure " << func_name << " Completes" << endl;
}

m_stat Measurer::_measure(function<double()> func, ostream &out, string file_name, int filters) {
    vector<double> results;
    for (int counter = 1; counter <= repeat; ++counter) {
        double result = func();
        results.push_back(result);
        if (out) {
            output_result(out, counter, result);
        }
        // Random sleep for 0.01 ~ 0.02 second
        usleep((1 + arc4random() % 2) * 10000);
    }
    
    m_stat s = get_stats(results);
    
    if (filters & MIN) {
        vector<double> tmp_results;
        for (double result : results) {
            if (result > 5 * s.min) {
                continue;
            }
            tmp_results.push_back(result);
        }
        results = tmp_results;
        s = get_stats(results);
    }
    
    if (filters & STD) {
        vector<double> tmp_results;
        for (double result : results) {
            if (result < s.mean - 3 * s.std || result > s.mean + 3 * s.std) {
                continue;
            }
            tmp_results.push_back(result);
        }
        results = tmp_results;
        s = get_stats(results);
    }
    
    if (file_name.length() > 0) {
        fstream file;
        file.open(file_name.data(), ios::out);
        int counter = 1;
        for (double result : results) {
            output_result(file, counter, result);
            counter++;
        }
        if (results.size() % 5 != 0) {
            file << endl;
        }
        file << endl << "Mean: " << s.mean << " Std: " << s.std << endl;
        file.close();
    }
    
    return s;
}
