//
//  Measurer.cpp
//  Project
//
//  Created by 范志康 on 2018/4/22.
//  Copyright © 2018年 范志康. All rights reserved.
//

#ifndef MEASURER_HPP
#define MEASURER_HPP

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <functional>
#include <math.h>
#include <numeric>
#include <vector>
#include "Config.hpp"

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

using namespace std;

static int repeat = 100;

#ifdef __APPLE__
inline uint64_t rdtsc() {
    return mach_absolute_time();
}
#endif

struct m_stat {
    double mean;
    double std;
    double min;
};

class Measurer {
public:
    enum FILTER_TYPE {
        STD = 1,
        MIN = 1 << 1,
    };
    
    static void measure(function<double()> func, string name, string type, int filters = STD);
    
    static void measure_multi(function<double(int)> func, vector<int> params, string func_name, string param_name, string type, int filters = STD);
    
private:
    static m_stat _measure(function<double()> func, ostream &out, string file_name, int filters);
};

#endif
