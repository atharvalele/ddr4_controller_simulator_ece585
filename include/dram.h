#ifndef __DRAM_H__
#define __DRAM_H__

#include <array>
#include <fstream>
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

    /* DRAM Commands File */
    std::ofstream& dram_cmd_file;

    /* DRAM Commands Type */
    typedef enum DRAM_COMMANDS {
        PRE,
        ACT,
        RD,
        WR,
        REF
    } dram_cmds_t;

    /* DRAM Bank and Bank Groups */
    static constexpr uint8_t BANKS = 4;
    static constexpr uint8_t BANK_GRPS = 4;

    /* DRAM Bank States */
    typedef enum DRAM_BANK_STATE {
        IDLE,
        PRECHARGE,    
        PRECHARGE_WAIT,
        PRECHARGED,
        ACTIVATE,
        ACTIVATE_WAIT,
        ACTIVATED,
        READ,
        READ_WAIT,
        WRITE,
        WRITE_WAIT,
        BURST
    } dram_bank_state_t;

    /* DRAM bank struct */
    struct dram_bank {
        dram_bank_state_t state;
        uint16_t timer;
        int32_t active_row = -1;
        int32_t req_row = -1;
        int32_t req_column = -1;
    };

    /* State of DRAM Banks */
    std::array<std::array<dram_bank, BANKS>, BANK_GRPS> bank;

    /*
     * Time since last bank group command
     * All of these will be saturating counters
     *
     */
    std::array<uint8_t, BANK_GRPS> time_since_bank_ACT;
    uint8_t LAST_ACTIVATED_BANK_GRP = 0xFF;

    std::array<uint8_t, BANK_GRPS> time_since_bank_RD;
    uint8_t LAST_READ_BANK_GRP = 0xFF;

    std::array<uint8_t, BANK_GRPS> time_since_bank_WR;
    uint8_t LAST_WRITTEN_BANK_GRP = 0xFF;

    dram_cmds_t LAST_COMMAND;

    /* Mark bus busy function */
    void mark_bus_busy(uint8_t clks);
    bool bus_busy {false};
    uint8_t bus_busy_timer = 0;

    /* DRAM FSM Trigger */
    void fsm_trigger();
    
    /* DRAM Bank State Machine */
    void bank_fsm(uint8_t bg, uint8_t b);

    /* Overload operator for printing state */
    friend std::ostream& operator<<(std::ostream& os, dram_bank &b)
    {
        os << "State: ";
        switch (b.state) {
            case IDLE:              os << "IDLE";               break;
            case PRECHARGE:         os << "PRECHARGE";          break;
            case PRECHARGED:        os << "PRECHARGED";         break;
            case PRECHARGE_WAIT:    os << "PRECHARGE_WAIT";     break;
            case ACTIVATE:          os << "ACTIVATE";           break;
            case ACTIVATE_WAIT:     os << "ACTIVATE_WAIT";      break;
            case ACTIVATED:         os << "ACTIVATED";          break;
            case READ:              os << "READ";               break;
            case READ_WAIT:         os << "READ_WAIT";          break;
            case WRITE:             os << "WRITE";              break;
            case WRITE_WAIT:        os << "WRITE_WAIT";         break;
            case BURST:             os << "BURST";              break;
            default:                os << "UNKNOWN/Update here"; break;
        }

        os << "\tTimer: " << std::dec << b.timer << std::endl;

        return os;
    }
    
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
    /* Constructor */
    DRAM(std::ofstream& dram_cmd_file);

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
    void read(uint64_t bank_group, uint64_t bank, uint64_t column);
    void write(uint64_t bank_group, uint64_t bank, uint64_t column);
    /* Extra Credit */
    void dram_refresh();
};

#endif