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

    // Prints corresponding CPU clock cycle, operation type, and HEX address of request
    friend std::ostream& operator<<(std::ostream& os, request &req)
    {
        os << "Time in queue: " << std::dec << req.q_time << "\tRequest type: " << req.req_type 
            << "\tAddress: 0x" << std::setw(9) << std::setfill('0') << std::uppercase << std::hex << req.address << std::endl;

        return os;
    }
};

#endif