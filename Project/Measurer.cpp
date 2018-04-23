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
#include <x86intrin.h>

using namespace std;

static string baseDir = "/Users/Frank/Documents/Code/CSE 221/Project/Project/Results/";

class Measurer {
public:
    static void measure(function<double ()> func, string name, int repeat = 200) {
        cout << "Measure " << name << " overhead begins" << endl;
        
        fstream file;
        file.open(baseDir + name + ".txt", ios::out);
        vector<double> results;
        for (int r = 1; r <= repeat; ++r) {
            double result = func();
            results.push_back(result);
            file << left << setw(5) << to_string(r) + ": ";
            file << left << setw(8) << result;
            if (r % 5 == 0) {
                file << endl;
            }
            // Random sleep for 0.2 ~ 0.5 second
            usleep((2 + arc4random() % 4) * 100000);
        }
        double mean = accumulate(results.begin(), results.end(), 0.0) / results.size();
        double var = 0.0;
        for_each(results.begin(), results.end(), [&](const double val) {
            var += pow((val - mean), 2);
        });
        double std = sqrt(var / (results.size() - 1));
        file << endl << "Mean: " << mean << " Std: " << std << endl;
        file.close();
        cout << "Measure " << name << " overhead completes" << endl;
    }
};
