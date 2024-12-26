#pragma once

#include <cstdint>
#include <vector>
#include <cstdint>

// Bit TWiddling functions
constexpr void wrapNort (uint64_t &bitBoard) {bitBoard <<= 8;}
constexpr void wrapSout (uint64_t &bitBoard) {bitBoard >>= 8;}
constexpr void wrapEast (uint64_t &bitBoard) {constexpr uint64_t mask = uint64_t(0x7f7f7f7f7f7f7f7f); bitBoard &= mask; bitBoard <<= 1;}
constexpr void wrapWest (uint64_t &bitBoard) {constexpr uint64_t mask = uint64_t(0xfefefefefefefefe); bitBoard &= mask; bitBoard >>= 1;}
constexpr uint64_t cpyWrapNort (uint64_t bitBoard) {wrapNort(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapSout (uint64_t bitBoard) {wrapSout(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapEast (uint64_t bitBoard) {wrapEast(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapWest (uint64_t bitBoard) {wrapWest(bitBoard); return bitBoard;}
int bitScanReverse (uint64_t bitBoard);
int bitScanForward (uint64_t bitBoard);

static int _popCount256[256];
static bool _isPopCountInit = false;
void initPopCount256();
int popCount(uint64_t bitBoard);