#include <sstream>

#include "dram.h"

/* Constructor */
DRAM::DRAM(std::ofstream &dram_cmd):
    dram_cmd_file(dram_cmd)
{
    /* Initialize all banks to precharged */
    for (uint8_t bg = 0; bg < BANK_GRPS; bg++) {
        for (uint8_t b = 0; b < BANKS; b++) {
            bank[bg][b].state = PRECHARGED;
            bank[bg][b].timer = 0;
        }
    }

    /* Initialize all time_since_XYZ counters */
    std::fill(time_since_bank_grp_ACT.begin(), time_since_bank_grp_ACT.end(), 255);
    std::fill(time_since_bank_grp_RD.begin(), time_since_bank_grp_RD.end(), 255);
    std::fill(time_since_bank_grp_WR.begin(), time_since_bank_grp_WR.end(), 255);
    std::fill(time_since_bank_grp_WR.begin(), time_since_bank_WR.end(), 255);
}

/* Add element to DRAM queue */
void DRAM::queue_add(request req)
{
    if (req_queue.size() < QUEUE_SIZE) {
        req_queue.push_back(req);
        std::cout << "Added to Queue: CPU Clock: " << std::dec << cpu_clock_tick << " - " << req;
    } else {
        std::cout << "Queue Full" << std::endl;
    }
}

/* Remove element from the queue */
void DRAM::queue_remove()
{
    if (!is_queue_empty()) {
        std::cout << "Removed from Queue: CPU Clock: " << std::dec << cpu_clock_tick << " - " << req_queue.front() << std::endl;
        req_queue.erase(req_queue.begin());
    }
}

/* Is queue empty? */
bool DRAM::is_queue_empty()
{
    return req_queue.empty();
}

/* Is queue full? */
bool DRAM::is_queue_full()
{
    return (req_queue.size() == QUEUE_SIZE);
}

/* DRAM FSM Trigger */
void DRAM::fsm_trigger()
{
    /* Start with the thing on top of the queue */
    if (!is_queue_empty()) {
        request req = req_queue[0];

        /* Ensure we don't trigger the same FSM twice */
        if (bank[req.bank_group][req.bank].state == ACTIVATED ||
            bank[req.bank_group][req.bank].state == PRECHARGED) {
            if (req.row == bank[req.bank_group][req.bank].active_row) {
                bank[req.bank_group][req.bank].req_column = req.col;
                /* Page Hit */
                /* Check if READ/WRITE and set state/timer accordingly */
                if (req.req_type == REQ_OP::READ || req.req_type == REQ_OP::FETCH) {
                    bank[req.bank_group][req.bank].state = READ;
                } else { /* WRITE */
                    bank[req.bank_group][req.bank].state = WRITE;
                }
            } else if (bank[req.bank_group][req.bank].active_row == -1) {
                /* Page Empty */
                bank[req.bank_group][req.bank].state = ACTIVATE;
                bank[req.bank_group][req.bank].req_row = req.row;
            } else {
                /* Page Miss */
                bank[req.bank_group][req.bank].state = PRECHARGE;
            }
        }
    }
}

/*
 * DRAM Bank State Machine
 *
 * Timers for intermediary wait states will be set to tXYZ - 1 values because
 * I am forcing them through go through each state of the FSM instead of the
 * FSM just bouncing around everywhere
 */
void DRAM::bank_fsm(uint8_t bg, uint8_t b)
{
    dram_bank& db = bank[bg][b];

    /* Decrement timer */
    if (db.timer > 0)
        db.timer--;

    /* Handle states */
    switch (db.state) {
    
    case IDLE:
        break;

    case PRECHARGE:
        /* Check for tWR or tRTP */
        if ((LAST_COMMAND == RD) && (time_since_bank_grp_RD[LAST_READ_BANK_GRP] < tRTP)) {
            break;
        } else if ((LAST_COMMAND == WR) && (LAST_WRITTEN_BANK == b) &&
                    (LAST_WRITTEN_BANK_GRP == bg) && (time_since_bank_WR[LAST_WRITTEN_BANK] <= (tCWD + tBURST-1 + tWR))) {
            break;
        }

        if (!bus_busy) {
            precharge(bg, b);
            db.timer = tRP - 1;
            db.state = PRECHARGE_WAIT;

            mark_bus_busy(1);
        }
        break;

    case PRECHARGE_WAIT:
        /* If done precharging, send to precharged state */
        if (!db.timer) {
            db.state = PRECHARGED;
            db.active_row = -1;
        }
        break;

    case PRECHARGED:
        /* Do nothing */
        break;
    
    case ACTIVATE:
        /* Check ACTIVATE timing constraints, and issue ACT */
        /* Check for tRRD_L / tRRD_S (Sanity...?) */
        if ((LAST_ACTIVATED_BANK_GRP == bg) && (time_since_bank_grp_ACT[bg] < tRRD_L)) {
            break;
        } else if ((LAST_ACTIVATED_BANK_GRP != bg) && (time_since_bank_grp_ACT[bg] < tRRD_S)) {
            break;
        }

        if (!bus_busy) {
            activate(bg, b, db.req_row);
            db.state = ACTIVATE_WAIT;
            db.timer = tRCD - 1;

            /* Update Last Activated Bank */
            LAST_ACTIVATED_BANK_GRP = bg;
            time_since_bank_grp_ACT[bg] = 0;

            LAST_COMMAND = ACT;

            mark_bus_busy(1);
        }
        break;

    case ACTIVATE_WAIT:
        if (!db.timer) {
            /* Update Activated row */
            db.active_row = db.req_row;
            db.state = ACTIVATED;
        }
        break;
    
    case ACTIVATED:
        break;

    case READ:
        if (LAST_COMMAND == RD) {
            /* Last command was a READ */
            /* Check for tCCD_L / tCCD_S (Less and less sanity...?) */
            if ((LAST_READ_BANK_GRP == bg) && (time_since_bank_grp_RD[bg] <= tCCD_L)) {
                break;
            } else if ((LAST_READ_BANK_GRP != bg) && (time_since_bank_grp_RD[bg] <= tCCD_S)) {
                break;
            }
        } else if (LAST_COMMAND == WR) {
            /* Last command was a WRITE */
            /* Check for tWTR_L / tWTR_S (Even lesser sanity...?) */
            if ((LAST_WRITTEN_BANK_GRP == bg) && (time_since_bank_grp_WR[bg] <= (tCWD + tBURST-1 + tWTR_L))) {
                break;
            } else if ((LAST_WRITTEN_BANK_GRP != bg) && (time_since_bank_grp_WR[bg] <= (tCWD + tBURST-1 + tWTR_S))) {
                break;
            }
        }

        if (!bus_busy) {
            /* Issue the read and set wait timings */
            read(bg, b, db.req_column);
            db.state = READ_WAIT;

            /* Data is valid *AFTER* tCAS, not *AT* tCAS */
            db.timer = tCAS;

            /* Update Last READ Bank */
            LAST_READ_BANK_GRP = bg;

            LAST_COMMAND = RD;

            /* Time since last read is counted from the issuance of RD */
            time_since_bank_grp_RD[bg] = 0;

            mark_bus_busy(1);

            queue_remove();
        }
        break;

    case READ_WAIT:
        if ((!db.timer) && (!bus_busy)) {
            db.state = BURST;

            /* Data is valid on bus for tBURST */
            db.timer = tBURST - 1;

            mark_bus_busy(tBURST);
        }
        break;

    case WRITE:
        if (LAST_COMMAND == RD) {
            /* Last command was a READ */
            /* Check for READ-to-WRITE timing */
            if (time_since_bank_grp_RD[LAST_READ_BANK_GRP] < (tCAS + tBURST - tCWD + 2))
                break;
        } else if (LAST_COMMAND == WR) {
            /* Last command was a WRITE */
            /* Check for tCCD_L / tCCD_S (Where sanity..? Monke.) */
            if ((LAST_WRITTEN_BANK_GRP == bg) && (time_since_bank_grp_WR[bg] <= tCCD_L)) {
                break;
            } else if ((LAST_WRITTEN_BANK_GRP != bg) && (time_since_bank_grp_WR[bg] <= tCCD_S)) {
                break;
            }
        }

        if (!bus_busy) {
            write(bg, b, bank[bg][b].req_column);
            db.state = WRITE_WAIT;

            /* Data is valid *AFTER* tCWD, not *AT* tCWD */
            db.timer = tCWD;

            /* Update Last WRITTEN Bank */
            LAST_WRITTEN_BANK_GRP = bg;
            LAST_WRITTEN_BANK = b;

            LAST_COMMAND = WR;

            time_since_bank_grp_WR[bg] = 0;
            time_since_bank_WR[b] = 0;

            mark_bus_busy(1);

            queue_remove();
        }
        break;

    case WRITE_WAIT:
        if ((!db.timer) && (!bus_busy)) {
            db.state = BURST;

            /* Data is valid on bus for tBURST */
            db.timer = tBURST - 1;

            mark_bus_busy(tBURST);
        }
        break;

    case BURST:
        if (!db.timer) {
            db.state = ACTIVATED;
        }
        break;

    default:
        break;
    }
}

/* DRAM operation */
void DRAM::do_ram_things()
{
    /* Trigger FSM */
    fsm_trigger();

    /* Run all bank state machines */
    for (uint8_t bg = 0; bg < BANK_GRPS; bg++) {
        for (uint8_t b = 0; b < BANKS; b++) {
            bank_fsm(bg, b);
        }
    }

    /* Increment Clock */
    clock_tick++;

    /* Handle bus busy */
    if (bus_busy) {
        bus_busy_timer--;

        if (!bus_busy_timer)
            bus_busy = false;
    }

    /* Increment time in queue for all */
    for (auto &r: req_queue) {
        r.q_time++;
    }

    /* Increment all time_since_last_XYZ counters */
    /* ACT */
    for (auto &t: time_since_bank_grp_ACT)
        if (t < 255)
            t++;
    
    /* READ */
    for (auto &t: time_since_bank_grp_RD)
        if (t < 255)
            t++;

    /* BANK GROUP WRITE */
    for (auto &t: time_since_bank_grp_WR)
        if (t < 255)
            t++;

    /* BANK WRITE */
    for (auto &t: time_since_bank_WR)
        if (t < 255)
            t++;
    
    /* PRECHARGE */
    for (auto &t: time_since_bank_grp_PRE)
        if (t < 255)
            t++;
}

/* Mark bus busy */
void DRAM::mark_bus_busy(uint8_t clks)
{
    bus_busy = true;
    bus_busy_timer = clks;
}

/* Advance Clocks */
void DRAM::clock_advance(uint64_t new_cpu_clock)
{
    /* Add 1 for odd division compensation? */
    uint64_t clock_diff = (new_cpu_clock - cpu_clock_tick) / 2;

    /* Increment clock */
    clock_tick += clock_diff;

    /* Increment all time_since_last_XYZ counters */
    /* ACT */
    for (auto &t: time_since_bank_grp_ACT) {
        if ((uint64_t)t + clock_diff > 255)
            t = 255;
        else
            t += clock_diff;
    }
    
    /* READ */
    for (auto &t: time_since_bank_grp_RD) {
        if ((uint64_t)t + clock_diff > 255)
            t = 255;
        else
            t += clock_diff;
    }

    /* BANK GROUP WRITE */
    for (auto &t: time_since_bank_grp_WR) {
        if ((uint64_t)t + clock_diff > 255)
            t = 255;
        else
            t += clock_diff;
    }

    /* BANK WRITE */
    for (auto &t: time_since_bank_WR) {
        if ((uint64_t)t + clock_diff > 255)
            t = 255;
        else
            t += clock_diff;
    }

    /* PRECHARGE */
    for (auto &t: time_since_bank_grp_PRE) {
        if ((uint64_t)t + clock_diff > 255)
            t = 255;
        else
            t += clock_diff;
    }
}

bool DRAM::is_time_jump_legal()
{
    /*
     * Iterate over all banks and if any one is not in the 
     * PRECHARGED or ACTIVATED state, the jump is illegal
     */
    for (uint8_t bg = 0; bg < BANK_GRPS; bg++) {
        for (uint8_t b = 0; b < BANKS; b++) {
            if ((bank[bg][b].state != PRECHARGED) && (bank[bg][b].state != ACTIVATED)) {
                    return false;
                }
        }
    }

    return true;
}

void DRAM::activate(uint64_t bank_group, uint64_t bank, uint64_t row)
{
    std::stringstream op;
    
    op << std::dec << cpu_clock_tick << "\t" << "ACT\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << row << std::endl;
    
    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::precharge(uint64_t bank_group, uint64_t bank)
{
    std::stringstream op;

    op << std::dec << cpu_clock_tick << "\t" << "PRE\t0x" << std::hex << bank_group << "\t0x" << bank << std::endl;
    
    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::read(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::stringstream op;

    op << std::dec << cpu_clock_tick << "\t" << "RD\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << column << std::endl;

    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::write(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::stringstream op;
    
    op << std::dec << cpu_clock_tick << "\t" << "WR\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << column << std::endl;

    std::cout << op.str();
    dram_cmd_file << op.str();
}
