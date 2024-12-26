#include <iostream>
#include <vector>
#include <cstdint>
#include "Move.hpp"
#include "utils.hpp"

int main(int argc, char **argv){
    std::vector<uint32_t> m_moveHist;
    m_moveHist.emplace_back(uint32_t(0x3f));
    m_moveHist.emplace_back(m_moveHist.back());
    m_moveHist.back() &= 0x0;
    std::cout<<m_moveHist[0]<< std::endl;
    return 0;
}