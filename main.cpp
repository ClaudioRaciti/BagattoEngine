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

    engine.getEval("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 11);

    // // Get ending timepoint
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    std::cout << "\nTime taken by function: "<< duration.count() << " seconds" << std::endl;
}