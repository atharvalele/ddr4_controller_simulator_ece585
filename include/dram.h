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

    /* State of Execution */
    typedef enum DRAM_STATE {
        IDLE = 0,
        PRE,
        PRE_WAIT,
        ACT,
        ACT_WAIT,
        READ,
        WRITE
    } OP_STATE;

    /* State of the FSM - Start with PRECHARGE */
    OP_STATE state = PRE;
    
    /* RAM parameters - in DRAM clock cycles */
    static constexpr uint8_t tRC = 76;          // ACTIVATE to ACTIVATE or REF command period 
    static constexpr uint8_t tRAS = 52;         // ACTIVATE to PRECHARGE command period
    static constexpr uint8_t tRRD_L = 6;        // ACTIVATE to ACTIVATE command period for same bank group
    static constexpr uint8_t tRRD_S = 4;        // ACTIVATE to ACTIVATE command period for different bank group
    static constexpr uint8_t tRP = 24;          // PRECHARGE command period
    // Add tRFC here
    static constexpr uint8_t tCWD = 20;         // CAS Write latency
    static constexpr uint8_t tCAS = 24;         // CAS Latency
    static constexpr uint8_t tRCD = 24;         // ACTIVATE to internal READ/WRITE delay
    static constexpr uint8_t tWR = 20;          // WRITE recovery
    static constexpr uint8_t tRTP = 12;         // READ to PRECHARGE command period
    static constexpr uint8_t tCCD_L = 8;        // READ to READ command period for same bank group
    static constexpr uint8_t tCCD_S = 4;        // READ to READ command period for different bank group
    static constexpr uint8_t tBURST = 4;        // BURST time
    static constexpr uint8_t tWTR_L = 12;       // WRITE to READ command period for same bank group
    static constexpr uint8_t tWTR_S = 4;        // WRITE to READ command period for different bank group
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
    void activate(uint64_t bank_group, uint64_t bank, uint64_t row);
    void precharge(uint64_t bank_group, uint64_t bank);
    void dram_read(uint64_t bank_group, uint64_t bank, uint64_t column);
    void dram_write(uint64_t bank_group, uint64_t bank, uint64_t column);
    /* Extra Credit */
    void dram_refresh();
};

#endif