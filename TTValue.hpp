#pragma once 

#include <cstdint>

class TTValue
{
private:
    uint32_t m_data; 
public:
    TTValue() : m_data{0U} {}
    TTValue(int16_t t_score, int t_depth, int t_nodeType) : 
        m_data{uint32_t(t_score & 0xffff) | uint32_t((t_depth & 0xff) << 16) | uint32_t((t_nodeType & 0xf) << 24)} {}
    ~TTValue() = default;
    inline bool operator==(const TTValue &t_other) const {return m_data == t_other.m_data;}

    constexpr int16_t score() const {return int16_t(m_data & 0xffff);}
    constexpr int nodeType()  const {return (m_data >> 24) & 0xf;}
    constexpr int depth()     const {return (m_data >> 16) & 0xff;}
};