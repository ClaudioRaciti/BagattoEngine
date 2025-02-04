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

    engine.getEval("r1bq1rk1/ppp1npp1/3p1n1p/4p3/1bB1P3/3P1NNP/PPP2PP1/R1BQ1RK1 b - - 3 9", 11);

    // // Get ending timepoint
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    std::cout << "\nTime taken by function: "<< duration.count() << " seconds" << std::endl;
}