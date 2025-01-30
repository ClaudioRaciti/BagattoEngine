#include "Engine.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <bitset>
using namespace std::chrono;


int main(){
    Engine engine;

    // // Get starting timepoint
    auto start = high_resolution_clock::now();

    std::cout << engine.getEval("rn4k1/p1pp2pp/1p2pr2/7q/3P1Pp1/2PB3N/PP1N1PK1/R2QR3 b - - 0 15", 12) << std::endl;

    // // Get ending timepoint
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    std::cout << "Time taken by function: "<< duration.count() << " seconds" << std::endl;
}