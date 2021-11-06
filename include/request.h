#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <iostream>

#include "commondefs.h"

// Class for the request queue
class request {
public:
    // Time spent by request in queue
    uint64_t q_time;
    // READ/WRITE/FETCH
    REQ_OP req_type;
    
    uint64_t address;

    // Prints corresponding CPU clock cycle, operation type, and HEX address of request
    friend std::ostream& operator<<(std::ostream& os, request &req)
    {
        os << "Time in queue: " << std::dec << req.q_time << "\tRequest type: " << req.req_type 
            << "\tAddress: 0x" << std::hex << req.address << std::endl;

        return os;
    }
};

#endif