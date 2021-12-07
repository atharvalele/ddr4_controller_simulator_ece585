#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <iostream>
#include <iomanip>

#include "commondefs.h"

// Class for the request queue
class request {
public:
    // CPU time at which memory request was made
    uint64_t cpu_req_time;
    // Time spent by request in queue
    uint64_t q_time;
    // READ/WRITE/FETCH
    REQ_OP req_type;
    
    uint64_t address;

    int32_t row;
    uint64_t high_col;
    uint64_t col;
    uint64_t bank;
    uint64_t bank_group;
    uint64_t burst_index;

    bool busy = false;
    bool valid = false;

    // Prints corresponding CPU clock cycle, operation type, and HEX address of request
    friend std::ostream& operator<<(std::ostream& os, request &req)
    {
        os << "Time in queue: " << std::dec << req.q_time << "\tRequest type: " << req.req_type 
            << "\tAddress: 0x" << std::setw(9) << std::setfill('0') << std::uppercase << std::hex << req.address << "\tRow: 0x" << req.row 
            << "\tHigh Column: 0x" << req.high_col << "\tBank: 0x" << req.bank << "\tBank Group: 0x" << req.bank_group << "\tBurst Index: 0x" << req.burst_index << std::endl;

        return os;
    }
};

#endif