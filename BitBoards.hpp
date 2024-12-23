#pragma once

#include <cstdint>

class BitBoards{
public:
    BitBoards();
    
    uint64_t getBitboard(int t_pieceType) const;

    int getKingSquare(int t_side) const;
    int getSideToMove() const;

    void setBitboard(int t_pieceType, uint64_t t_bitBoard);
    void toggleSideToMove();
    
private:
    int m_sideToMove;
    int m_kingSquare[2];
    uint64_t m_bitBoard[8];
};
