#include "Debugger.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <bitset>
#include "HashTable.hpp"
using namespace std::chrono;


int main(){
    Debugger debugger;

    // Get starting timepoint
    auto start = high_resolution_clock::now();

    std::cout << debugger.getPerft(6) << std::endl;

    // Get ending timepoint
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    std::cout << "Time taken by function: "<< duration.count() << " milliseconds" << std::endl;
}