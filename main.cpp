#include "Debugger.hpp"
#include <chrono>
#include <iostream>
using namespace std::chrono;

int main(int argc, char **argv){
    Debugger debugger("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");

    // Get starting timepoint
    auto start = high_resolution_clock::now();

    std::cout << debugger.getPerft(6) << std::endl;

    // Get ending timepoint
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    std::cout << "Time taken by function: "<< duration.count() << " milliseconds" << std::endl;

    return 0;
}