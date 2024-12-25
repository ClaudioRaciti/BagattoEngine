#include <iostream>
#include <cstdint>
#include "Move.hpp"

int main(int argc, char **argv){
    Move better(a1, b1, capture, rook, pawn);
    Move worse(a1, b1, capture, queen, pawn);
    std::cout  << better << better.asInt() << std::endl;
    std::cout  << worse  << worse.asInt() << std::endl; 

    if(better.asInt() >  worse.asInt()) std::cout << "It works!"<< std::endl; 


    return 0;
}