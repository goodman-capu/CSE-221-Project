//
//  Measurer.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <math.h>
#include <unistd.h>

using namespace std;

static string baseDir = "/Users/Frank/Documents/Code/CSE 221/Project/Project/Results/";

class Measurer {
public:
    static void measure(double (*func)(), string name, int repeat = 20) {
        cout << "Measure " << name << " begins" << endl;
        
        fstream file;
        file.open(baseDir + name + ".txt", ios::out);
        vector<double> results;
        for (int r = 1; r <= repeat; ++r) {
            double result = func();
            results.push_back(result);
            file << r << '\t' << result << endl;
            // Random sleep for 1 ~ 3 second
            usleep((1 + arc4random() % 3) * 1000000);
        }
        double mean = accumulate(results.begin(), results.end(), 0.0) / results.size();
        double var = 0.0;
        for_each(results.begin(), results.end(), [&](const double val) {
            var += pow((val - mean), 2);
        });
        double std = sqrt(var / (results.size() - 1));
        file << endl << "Mean: " << mean << " Std: " << std << endl;
        file.close();
        cout << "Measure " << name << " completes" << endl;
    }
};
