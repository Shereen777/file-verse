#ifndef Index_HPP
#define Index_HPP

#include "../include/odf_types.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;

// ============================================================================
// RANDOM INDEX GENERATOR
// ============================================================================

class IndexGenerator {
private:
    static mt19937 rng;
    static uniform_int_distribution<uint32_t> dist;
    
public:
    static void initialize() {
        rng.seed(time(nullptr));
    }
    
    static uint32_t generate() {
        return dist(rng);
    }
};

#endif