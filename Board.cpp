#include "Board.hpp"

Board::Board()
{
    m_stateHist.reserve(20);
    m_stateHist.emplace_back(uint32_t(0x1e));
}