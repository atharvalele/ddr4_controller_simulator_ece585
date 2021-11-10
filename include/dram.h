#ifndef __DRAM_H__
#define __DRAM_H__

#include <vector>

#include "commondefs.h"
#include "request.h"

class DRAM {
private:
    /* DRAM Clock Cycles */
    uint64_t clock_tick = 0;
    
    /* Queue */
    static constexpr uint8_t QUEUE_SIZE = 16;
    std::vector<request> req_queue;

    /* RAM parameters - in DRAM clock cycles */
    static constexpr uint8_t tRC = 76;          // ACTIVATE to ACTIVATE or REF command period 
    static constexpr uint8_t tRAS = 52;         // ACTIVATE to PRECHARGE command period
    static constexpr uint8_t tRRD_L = 6;        // ACTIVATE to ACTIVATE command period for same bank group
    static constexpr uint8_t tRRD_S = 4;        // ACTIVATE to ACTIVATE command period for same bank group
    static constexpr uint8_t tRP = 24;          // PRECHARGE command period
    // Add tRFC here
    static constexpr uint8_t tCWD = 20;         // Not sure yet
    static constexpr uint8_t tCAS = 24;
    static constexpr uint8_t tRCD = 24;         // ACTIVATE to internal READ/WRITE delay
    static constexpr uint8_t tWR = 20;
    static constexpr uint8_t tRTP = 12;
    static constexpr uint8_t tCCD_L = 8;
    static constexpr uint8_t tCCD_S = 4;
    static constexpr uint8_t tBURST = 4;
    static constexpr uint8_t tWTR_L = 12;
    static constexpr uint8_t tWTR_S = 4;
    // Add refresh time here

public:
    /* DRAM Clock Freq (MHz) */
    static constexpr uint16_t DRAM_CLK_FREQ = 1600;

    /* Queue Methods */
    void queue_add(request req);
    void queue_remove();
    bool is_queue_empty();
    bool is_queue_full();

    /* RAM Operation */
    void do_ram_things();
};

#endif