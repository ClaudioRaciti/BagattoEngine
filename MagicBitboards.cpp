#include "MagicBitboards.hpp"
#include "utils.hpp"
#include "notation.hpp"

MagicBitboards* MagicBitboards::m_instance = nullptr;

const MagicBitboards &MagicBitboards::getInstance()
{
    if (m_instance == nullptr) m_instance = new MagicBitboards();
    return *m_instance;
}

// uint64_t MagicBitboards::getAttacks(int t_piece, int t_square, uint64_t t_occupied) const
// {
//     using AttackFunction = uint64_t (MagicBitboards::*)(uint64_t, int) const;

//     static const AttackFunction attackFunction[] = {
//         &MagicBitboards::knightAttacks, // knight
//         &MagicBitboards::bishopAttacks, // bishops
//         &MagicBitboards::rookAttacks,   // rooks
//         &MagicBitboards::queenAttacks,  // queens
//         &MagicBitboards::kingAttacks    // kings
//     };
    
//     return (m_instance->*attackFunction[t_piece - 3])(t_occupied, t_square);
// }

MagicBitboards::MagicBitboards() :
   m_rShift {
        52, 53, 53, 53, 53, 53, 53, 52,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 53, 53, 53, 53, 53
    } ,
    m_rMask {
        uint64_t(0x000101010101017E), uint64_t(0x000202020202027C), uint64_t(0x000404040404047A), uint64_t(0x0008080808080876),
        uint64_t(0x001010101010106E), uint64_t(0x002020202020205E), uint64_t(0x004040404040403E), uint64_t(0x008080808080807E),
        uint64_t(0x0001010101017E00), uint64_t(0x0002020202027C00), uint64_t(0x0004040404047A00), uint64_t(0x0008080808087600),
        uint64_t(0x0010101010106E00), uint64_t(0x0020202020205E00), uint64_t(0x0040404040403E00), uint64_t(0x0080808080807E00),
        uint64_t(0x00010101017E0100), uint64_t(0x00020202027C0200), uint64_t(0x00040404047A0400), uint64_t(0x0008080808760800),
        uint64_t(0x00101010106E1000), uint64_t(0x00202020205E2000), uint64_t(0x00404040403E4000), uint64_t(0x00808080807E8000),
        uint64_t(0x000101017E010100), uint64_t(0x000202027C020200), uint64_t(0x000404047A040400), uint64_t(0x0008080876080800),
        uint64_t(0x001010106E101000), uint64_t(0x002020205E202000), uint64_t(0x004040403E404000), uint64_t(0x008080807E808000),
        uint64_t(0x0001017E01010100), uint64_t(0x0002027C02020200), uint64_t(0x0004047A04040400), uint64_t(0x0008087608080800),
        uint64_t(0x0010106E10101000), uint64_t(0x0020205E20202000), uint64_t(0x0040403E40404000), uint64_t(0x0080807E80808000),
        uint64_t(0x00017E0101010100), uint64_t(0x00027C0202020200), uint64_t(0x00047A0404040400), uint64_t(0x0008760808080800),
        uint64_t(0x00106E1010101000), uint64_t(0x00205E2020202000), uint64_t(0x00403E4040404000), uint64_t(0x00807E8080808000),
        uint64_t(0x007E010101010100), uint64_t(0x007C020202020200), uint64_t(0x007A040404040400), uint64_t(0x0076080808080800),
        uint64_t(0x006E101010101000), uint64_t(0x005E202020202000), uint64_t(0x003E404040404000), uint64_t(0x007E808080808000),
        uint64_t(0x7E01010101010100), uint64_t(0x7C02020202020200), uint64_t(0x7A04040404040400), uint64_t(0x7608080808080800),
        uint64_t(0x6E10101010101000), uint64_t(0x5E20202020202000), uint64_t(0x3E40404040404000), uint64_t(0x7E80808080808000)
    },
    m_rMagic {
        uint64_t(0x0080001020400080), uint64_t(0x0040001000200040), uint64_t(0x0080081000200080), uint64_t(0x0080040800100080),
        uint64_t(0x0080020400080080), uint64_t(0x0080010200040080), uint64_t(0x0080008001000200), uint64_t(0x0080002040800100),
        uint64_t(0x0000800020400080), uint64_t(0x0000400020005000), uint64_t(0x0000801000200080), uint64_t(0x0000800800100080),
        uint64_t(0x0000800400080080), uint64_t(0x0000800200040080), uint64_t(0x0000800100020080), uint64_t(0x0000800040800100),
        uint64_t(0x0000208000400080), uint64_t(0x0000404000201000), uint64_t(0x0000808010002000), uint64_t(0x0000808008001000),
        uint64_t(0x0000808004000800), uint64_t(0x0000808002000400), uint64_t(0x0000010100020004), uint64_t(0x0000020000408104),
        uint64_t(0x0000208080004000), uint64_t(0x0000200040005000), uint64_t(0x0000100080200080), uint64_t(0x0000080080100080),
        uint64_t(0x0000040080080080), uint64_t(0x0000020080040080), uint64_t(0x0000010080800200), uint64_t(0x0000800080004100),
        uint64_t(0x0000204000800080), uint64_t(0x0000200040401000), uint64_t(0x0000100080802000), uint64_t(0x0000080080801000),
        uint64_t(0x0000040080800800), uint64_t(0x0000020080800400), uint64_t(0x0000020001010004), uint64_t(0x0000800040800100),
        uint64_t(0x0000204000808000), uint64_t(0x0000200040008080), uint64_t(0x0000100020008080), uint64_t(0x0000080010008080),
        uint64_t(0x0000040008008080), uint64_t(0x0000020004008080), uint64_t(0x0000010002008080), uint64_t(0x0000004081020004),
        uint64_t(0x0000204000800080), uint64_t(0x0000200040008080), uint64_t(0x0000100020008080), uint64_t(0x0000080010008080),
        uint64_t(0x0000040008008080), uint64_t(0x0000020004008080), uint64_t(0x0000800100020080), uint64_t(0x0000800041000080),
        uint64_t(0x00FFFCDDFCED714A), uint64_t(0x007FFCDDFCED714A), uint64_t(0x003FFFCDFFD88096), uint64_t(0x0000040810002101),
        uint64_t(0x0001000204080011), uint64_t(0x0001000204000801), uint64_t(0x0001000082000401), uint64_t(0x0001FFFAABFAD1A2)
    },
    m_bShift {
        58, 59, 59, 59, 59, 59, 59, 58,
        59, 59, 59, 59, 59, 59, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 59, 59, 59, 59, 59, 59,
        58, 59, 59, 59, 59, 59, 59, 58
    },
    m_bMask {
        uint64_t(0x0040201008040200), uint64_t(0x0000402010080400), uint64_t(0x0000004020100A00), uint64_t(0x0000000040221400),
        uint64_t(0x0000000002442800), uint64_t(0x0000000204085000), uint64_t(0x0000020408102000), uint64_t(0x0002040810204000),
        uint64_t(0x0020100804020000), uint64_t(0x0040201008040000), uint64_t(0x00004020100A0000), uint64_t(0x0000004022140000),
        uint64_t(0x0000000244280000), uint64_t(0x0000020408500000), uint64_t(0x0002040810200000), uint64_t(0x0004081020400000),
        uint64_t(0x0010080402000200), uint64_t(0x0020100804000400), uint64_t(0x004020100A000A00), uint64_t(0x0000402214001400),
        uint64_t(0x0000024428002800), uint64_t(0x0002040850005000), uint64_t(0x0004081020002000), uint64_t(0x0008102040004000),
        uint64_t(0x0008040200020400), uint64_t(0x0010080400040800), uint64_t(0x0020100A000A1000), uint64_t(0x0040221400142200),
        uint64_t(0x0002442800284400), uint64_t(0x0004085000500800), uint64_t(0x0008102000201000), uint64_t(0x0010204000402000),
        uint64_t(0x0004020002040800), uint64_t(0x0008040004081000), uint64_t(0x00100A000A102000), uint64_t(0x0022140014224000),
        uint64_t(0x0044280028440200), uint64_t(0x0008500050080400), uint64_t(0x0010200020100800), uint64_t(0x0020400040201000),
        uint64_t(0x0002000204081000), uint64_t(0x0004000408102000), uint64_t(0x000A000A10204000), uint64_t(0x0014001422400000),
        uint64_t(0x0028002844020000), uint64_t(0x0050005008040200), uint64_t(0x0020002010080400), uint64_t(0x0040004020100800),
        uint64_t(0x0000020408102000), uint64_t(0x0000040810204000), uint64_t(0x00000A1020400000), uint64_t(0x0000142240000000),
        uint64_t(0x0000284402000000), uint64_t(0x0000500804020000), uint64_t(0x0000201008040200), uint64_t(0x0000402010080400),
        uint64_t(0x0002040810204000), uint64_t(0x0004081020400000), uint64_t(0x000A102040000000), uint64_t(0x0014224000000000),
        uint64_t(0x0028440200000000), uint64_t(0x0050080402000000), uint64_t(0x0020100804020000), uint64_t(0x0040201008040200)
    },
    m_bMagic {
        uint64_t(0x0002020202020200), uint64_t(0x0002020202020000), uint64_t(0x0004010202000000), uint64_t(0x0004040080000000),
        uint64_t(0x0001104000000000), uint64_t(0x0000821040000000), uint64_t(0x0000410410400000), uint64_t(0x0000104104104000),
        uint64_t(0x0000040404040400), uint64_t(0x0000020202020200), uint64_t(0x0000040102020000), uint64_t(0x0000040400800000),
        uint64_t(0x0000011040000000), uint64_t(0x0000008210400000), uint64_t(0x0000004104104000), uint64_t(0x0000002082082000),
        uint64_t(0x0004000808080800), uint64_t(0x0002000404040400), uint64_t(0x0001000202020200), uint64_t(0x0000800802004000),
        uint64_t(0x0000800400A00000), uint64_t(0x0000200100884000), uint64_t(0x0000400082082000), uint64_t(0x0000200041041000),
        uint64_t(0x0002080010101000), uint64_t(0x0001040008080800), uint64_t(0x0000208004010400), uint64_t(0x0000404004010200),
        uint64_t(0x0000840000802000), uint64_t(0x0000404002011000), uint64_t(0x0000808001041000), uint64_t(0x0000404000820800),
        uint64_t(0x0001041000202000), uint64_t(0x0000820800101000), uint64_t(0x0000104400080800), uint64_t(0x0000020080080080),
        uint64_t(0x0000404040040100), uint64_t(0x0000808100020100), uint64_t(0x0001010100020800), uint64_t(0x0000808080010400),
        uint64_t(0x0000820820004000), uint64_t(0x0000410410002000), uint64_t(0x0000082088001000), uint64_t(0x0000002011000800),
        uint64_t(0x0000080100400400), uint64_t(0x0001010101000200), uint64_t(0x0002020202000400), uint64_t(0x0001010101000200),
        uint64_t(0x0000410410400000), uint64_t(0x0000208208200000), uint64_t(0x0000002084100000), uint64_t(0x0000000020880000),
        uint64_t(0x0000001002020000), uint64_t(0x0000040408020000), uint64_t(0x0004040404040000), uint64_t(0x0002020202020000),
        uint64_t(0x0000104104104000), uint64_t(0x0000002082082000), uint64_t(0x0000000020841000), uint64_t(0x0000000000208800),
        uint64_t(0x0000000010020200), uint64_t(0x0000000404080200), uint64_t(0x0000040404040400), uint64_t(0x0002020202020200)
    }
{
    initRayAttacks();
    initKnightAttacks();
    initKingAttacks();
    initPawnAttacks();
    initMagicMoves();
}

void MagicBitboards::initRayAttacks()
{
    uint64_t nortRay =  (uint64_t) 0x0101010101010100;
    for (int sq = 0; sq < 64; sq ++, nortRay <<= 1)
        m_rayAttacks[sq][nort] = nortRay;


    uint64_t noEaRay = (uint64_t) 0x8040201008040200;
    for (int file = 0; file < 8; file ++, wrapEast(noEaRay)){
        uint64_t wrappedRay = noEaRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noEa] = wrappedRay;
    }

    uint64_t noWeRay = (uint64_t) 0x102040810204000;
    for (int file = 7; file >= 0; file --, wrapWest(noWeRay)){
        uint64_t wrappedRay = noWeRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noWe] = wrappedRay;
    }

    uint64_t eastRay = (uint64_t) 0x0000000000000fe;
    for (int file = 0; file < 8; file ++, wrapEast(eastRay)){
        uint64_t wrappedRay = eastRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][east] = wrappedRay;
    }

    uint64_t soutRay = (uint64_t) 0x0080808080808080;
    for (int sq = 63; sq >= 0; sq --, soutRay >>= 1)
        m_rayAttacks[sq][sout] = soutRay;

    uint64_t soEaRay = (uint64_t) 0x0002040810204080;
    for (int file = 0; file < 8; file ++, wrapEast(soEaRay)){
        uint64_t wrappedRay = soEaRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soEa] = wrappedRay;
    }

    uint64_t soWeRay = (uint64_t) 0x0040201008040201;
    for (int file = 7; file >= 0; file --, wrapWest(soWeRay)){
        uint64_t wrappedRay = soWeRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soWe] = wrappedRay;
    }

    uint64_t westRay = (uint64_t) 0x7f00000000000000;
    for (int file = 7; file >= 0; file --, wrapWest(westRay)){
        uint64_t wrappedRay = westRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][west] = wrappedRay;
    }
}

void MagicBitboards::initKnightAttacks()
{
    uint64_t knightPos = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, knightPos <<= 1){
        m_knightAttacks[sq] = (uint64_t) 0;

        uint64_t eastDir = cpyWrapEast(knightPos);
        uint64_t eaEaDir = cpyWrapEast(eastDir);
        m_knightAttacks[sq] |= eastDir << 16 | eaEaDir << 8 
            | eastDir >> 16 | eaEaDir >> 8;

        uint64_t westDir = cpyWrapWest(knightPos);
        uint64_t weWeDir = cpyWrapWest(westDir);
        m_knightAttacks[sq] |= westDir << 16 | weWeDir << 8
            | westDir >> 16 | weWeDir >> 8;
    }
}

void MagicBitboards::initKingAttacks()
{
    uint64_t kingPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, kingPosition <<= 1){
        m_kingAttacks[sq] = kingPosition << 8 | kingPosition >> 8;
        uint64_t eastDir = cpyWrapEast(kingPosition);
        m_kingAttacks[sq] |= eastDir | eastDir << 8 | eastDir >> 8;
        uint64_t westDir = cpyWrapWest(kingPosition);
        m_kingAttacks[sq] |= westDir | westDir << 8 | westDir >> 8;
    }
}

void MagicBitboards::initPawnAttacks()
{
    uint64_t pawnPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++ , pawnPosition <<= 1){
        uint64_t nortDir = pawnPosition << 8;
        uint64_t soutDir = pawnPosition >> 8;

        m_pawnAttacks[sq][white] = cpyWrapEast(nortDir) | cpyWrapWest(nortDir);
        m_pawnAttacks[sq][black] = cpyWrapEast(soutDir) | cpyWrapWest(soutDir);
    }    
}

void MagicBitboards::initMagicMoves()
{
    for (int i = 0; i < 64; i++){
        int squares[64];
        int numSquares = 0;
        uint64_t tmp = m_bMask[i];

        if (tmp) do {
            squares[numSquares ++] = bitScanForward(tmp);
        } while (tmp &= (tmp - 1));

        for(uint64_t occSeq = 0; occSeq < ((uint64_t) 1 << numSquares); occSeq++){
            uint64_t tmpOcc = initMagicOcc(squares, numSquares, occSeq);
            m_bMagicDb[i][(tmpOcc * m_bMagic[i]) >> m_bShift[i]] = initMagicBMoves(i,tmpOcc);
            // without applying the mask
            // the for loop should ensure that all combination of active masked bits are computed and
            // stored in the Magic database. The funcion `initMagicOcc` returns the right bit combination
            // for a given sequence of on and off bits in the mask (represented by occSeq, wich stores the
            // bit sequence, but each bit is not in the right place of the bitboard)
        }
    }

    for (int i = 0; i < 64; i++){
        int squares[64];
        int numSquares = 0;
        uint64_t tmp = m_rMask[i];

        if (tmp) do {
            squares[numSquares ++] = bitScanForward(tmp);
        } while (tmp &= (tmp - 1));

        for(uint64_t occSeq = 0;occSeq < ((uint64_t) 1 << numSquares); occSeq++){
            uint64_t tmpOcc = initMagicOcc(squares, numSquares, occSeq);
            m_rMagicDb[i][(tmpOcc * m_rMagic[i]) >> m_rShift[i]] = initMagicRMoves(i,tmpOcc);
        }
    }
}

uint64_t MagicBitboards::initMagicOcc(int *squares, int nSquares, uint64_t sequence)
{
    uint64_t returnVal = 0;
    for (int i = 0; i < nSquares; i ++)
        if (sequence & ((uint64_t) 1 << i)) returnVal |= (uint64_t) 1 << squares[i];
    return returnVal;
}

uint64_t MagicBitboards::initMagicBMoves(int t_square, uint64_t t_occupied)
{
    uint64_t ret=0;
	uint64_t bit;
	uint64_t bit2;
	uint64_t rowbits=(((uint64_t)0xFF)<<(8*(t_square/8)));
	
	bit=(((uint64_t)(1))<<t_square);
	bit2=bit;
	do
	{
		bit<<=8-1;
		bit2>>=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&t_occupied));
	bit=(((uint64_t)(1))<<t_square);
	bit2=bit;
	do
	{
		bit<<=8+1;
		bit2<<=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&t_occupied));
	bit=(((uint64_t)(1))<<t_square);
	bit2=bit;
	do
	{
		bit>>=8-1;
		bit2<<=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&t_occupied));
	bit=(((uint64_t)(1))<<t_square);
	bit2=bit;
	do
	{
		bit>>=8+1;
		bit2>>=1;
		if(bit2&rowbits) ret|=bit;
		else break;
	}while(bit && !(bit&t_occupied));
	return ret;
}

uint64_t MagicBitboards::initMagicRMoves(int t_square, uint64_t t_occupied)
{
    uint64_t ret=0;
	uint64_t bit;
	uint64_t rowbits=(((uint64_t)0xFF)<<(8*(t_square/8)));
	
	bit=(((uint64_t)(1))<<t_square);
	do
	{
		bit<<=8;
		ret|=bit;
	}while(bit && !(bit&t_occupied));
	bit=(((uint64_t)(1))<<t_square);
	do
	{
		bit>>=8;
		ret|=bit;
	}while(bit && !(bit&t_occupied));
	bit=(((uint64_t)(1))<<t_square);
	do
	{
		bit<<=1;
		if(bit&rowbits) ret|=bit;
		else break;
	}while(!(bit&t_occupied));
	bit=(((uint64_t)(1))<<t_square);
	do
	{
		bit>>=1;
		if(bit&rowbits) ret|=bit;
		else break;
	}while(!(bit&t_occupied));
	return ret;
}