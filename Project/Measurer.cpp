//
//  Measurer.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <vector>
#include <numeric>
#include <math.h>
#include <unistd.h>
#include <mach/mach_time.h>
#include "Config.cpp"

using namespace std;

static int repeat = 200;

inline uint64_t rdtsc() {
    return mach_absolute_time();
}

class Measurer {
public:
    enum FILTER_TYPE {
        STD = 1,
        MIN = 1 << 1
    };
    
    static void measure(function<double ()> func, string name, int filters = STD) {
        cout << "Measure " << name << " overhead begins" << endl;
        
        vector<double> results;
        for (int counter = 1; counter <= repeat; ++counter) {
            double result = func();
            results.push_back(result);
            outputResult(cout, counter, result);
            // Random sleep for 0.05 ~ 0.1 second
            usleep((1 + arc4random() % 2) * 50000);
        }
        
        fstream file;
        file.open(baseDir + name + ".txt", ios::out);
        
        vector<double> stats;
        double mean, std, min;
        
        stats = getStats(results);
        mean = stats[0];
        std = stats[1];
        min = stats[2];
        
        if (filters & MIN) {
            vector<double> tmpResults;
            for (double result : results) {
                if (result > 5 * min) {
                    continue;
                }
                tmpResults.push_back(result);
            }
            results = tmpResults;
            stats = getStats(results);
            mean = stats[0];
            std = stats[1];
            min = stats[2];
        }
        
        if (filters & STD) {
            vector<double> tmpResults;
            for (double result : results) {
                if (result < mean - 3 * std || result > mean + 3 * std) {
                    continue;
                }
                tmpResults.push_back(result);
            }
            results = tmpResults;
            stats = getStats(results);
            mean = stats[0];
            std = stats[1];
            min = stats[2];
        }
        
        int counter = 1;
        for (double result : results) {
            outputResult(file, counter, result);
            counter++;
        }
        if (results.size() % 5 != 0) {
            file << endl;
        }
        file << endl << "Mean: " << mean << " Std: " << std << endl;
        file.close();
        
        cout << "Measure " << name << " overhead completes" << endl;
    }
    
private:
    static vector<double> getStats(vector<double> &results) {
        double mean = accumulate(results.begin(), results.end(), 0.0) / results.size();
        double var = 0.0;
        for_each(results.begin(), results.end(), [&](const double val) {
            var += pow((val - mean), 2);
        });
        double std = sqrt(var / (results.size() - 1));
        double min = *min_element(results.begin(), results.end());
        return {mean, std, min};
    }
    
    static void outputResult(ostream &os, int counter, double result) {
        os << left << setw(5) << to_string(counter) + ": ";
        os << left << setw(12) << result;
        if (counter % 5 == 0) {
            os << endl;
        }
    }
};
