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

    int eval = engine.getEval("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 11);

    std::cout << "Mate in " << (- eval - CHECKMATE + 1) / 2  << std::endl;

    // // Get ending timepoint
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    std::cout << "\nTime taken by function: "<< duration.count() << " seconds" << std::endl;
}