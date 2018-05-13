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
#include <mach/mach_time.h>
#include "Config.hpp"

using namespace std;

static int repeat = 200;

inline uint64_t rdtsc() {
    return mach_absolute_time();
}

class Measurer {
public:
    enum FILTER_TYPE {
        STD = 1,
        MIN = 1 << 1,
    };
    
    static void measure_overhead(function<double ()> func, string name, int filters = STD);
};

#endif
