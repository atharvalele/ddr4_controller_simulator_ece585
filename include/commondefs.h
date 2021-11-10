#ifndef __COMMONDEF_H__
#define __COMMONDEF_H__

#include <ostream>

// CPU Clock Frequency (MHz)
constexpr uint16_t CPU_CLK_FREQ = 3200;

// CPU Clock tick counter
extern uint64_t cpu_clock_tick;

// Memory request type enumerator
enum REQ_OP
{
    READ,
    WRITE,
    FETCH
};

// Overload for REQ_OP cout
inline std::ostream& operator<<(std::ostream& os, REQ_OP req)
{
    switch (req)
    {
        case 0: os << "READ"; break;
        case 1: os << "WRITE"; break;
        case 2: os << "FETCH"; break;
        default: os << "UNKWN"; break;
    }
    return os;
}

#endif