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
#include "Config.cpp"

using namespace std;

class Measurer {
public:
    static void measure(function<double ()> func, string name, int repeat = 200) {
        cout << "Measure " << name << " overhead begins" << endl;
        
        vector<double> rawResults, results;
        for (int counter = 1; counter <= repeat; ++counter) {
            double result = func();
            rawResults.push_back(result);
            outputResult(cout, counter, result);
            // Random sleep for 0.2 ~ 0.5 second
            usleep((2 + arc4random() % 4) * 100000);
        }
        
        fstream file;
        file.open(baseDir + name + ".txt", ios::out);
        pair<double, double> meanAndStd = getMeanAndStd(rawResults);
        double mean = meanAndStd.first;
        double std = meanAndStd.second;
        int counter = 1;
        for (double result : rawResults) {
            if (result < mean - 3 * std || result > mean + 3 * std) {
                continue;
            }
            results.push_back(result);
            outputResult(file, counter, result);
            counter++;
        }
        if (results.size() % 5 != 0) {
            file << endl;
        }
        
        meanAndStd = getMeanAndStd(results);
        mean = meanAndStd.first;
        std = meanAndStd.second;
        file << endl << "Mean: " << mean << " Std: " << std << endl;
        file.close();
        cout << "Measure " << name << " overhead completes" << endl;
    }
    
private:
    static pair<double, double> getMeanAndStd(vector<double> &results) {
        double mean = accumulate(results.begin(), results.end(), 0.0) / results.size();
        double var = 0.0;
        for_each(results.begin(), results.end(), [&](const double val) {
            var += pow((val - mean), 2);
        });
        double std = sqrt(var / (results.size() - 1));
        return {mean, std};
    }
    
    static void outputResult(ostream &os, int counter, double result) {
        os << left << setw(5) << to_string(counter) + ": ";
        os << left << setw(8) << result;
        if (counter % 5 == 0) {
            os << endl;
        }
    }
};
