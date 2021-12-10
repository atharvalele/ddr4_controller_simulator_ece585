#include <iomanip>
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
    std::fill(time_since_bank_WR.begin(), time_since_bank_WR.end(), 255);
}

/* Look for the queue element */
int8_t DRAM::queue_search_active_req(uint64_t address)
{
    int8_t pos = -1;
    for (size_t i = 0; i < req_queue.size(); i++) {
        if ((req_queue[i].address == address) && (req_queue[i].busy == true)) {
            pos = i;
            break;
        }
    }

    return pos;
}

/* Add element to DRAM queue */
void DRAM::queue_add(request req)
{
    if (req_queue.size() < QUEUE_SIZE) {
        #ifdef RESCHEDULING
        int8_t pos = 0;
        if (is_req_page_active(req)) {
            pos = queue_get_free_location();
            //std::cout << "Free Location: " << (int)pos << std::endl;
        }
        auto iterator_pos = req_queue.begin() + pos;
        req_queue.insert(iterator_pos, req);
        #else
        req_queue.push_back(req);
        #endif
        #ifdef DEBUG
        std::cout << "Added to Queue: CPU Clock: " << std::dec << cpu_clock_tick << " - " << req;
        #endif
    } else {
        #ifdef DEBUG
        std::cout << "Queue Full" << std::endl;
        #endif
    }
}

/* Remove element from the queue */
void DRAM::queue_remove(uint64_t address)
{
    int8_t pos;

    if (!is_queue_empty()) {
        #ifdef BANK_PARALLELISM
        pos = queue_search_active_req(address);
        #else
        pos = 0;
        #endif
        if (pos != -1) {
            #ifdef DEBUG
            std::cout << "Removed from Queue: CPU Clock: " << std::dec << cpu_clock_tick << " - " << req_queue[pos] << std::endl;
            #endif
            req_queue.erase(req_queue.begin() + pos);
        } else {
            std::cerr << "Element not found! ERROR" << std::endl;
            exit(1);
        }
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

#ifdef BANK_PARALLELISM
/* DRAM FSM Trigger */
void DRAM::fsm_trigger()
{
    /* Iterate over all requests in queue that are not busy */
    for (auto &req: req_queue) {
        if (req.busy == false) {
            if (bank[req.bank_group][req.bank].busy == false) {
                req.busy = true;
                bank[req.bank_group][req.bank].busy = true;
                bank[req.bank_group][req.bank].address = req.address;
                bank[req.bank_group][req.bank].req_row = req.row;
                bank[req.bank_group][req.bank].req_column = req.col;

                bank[req.bank_group][req.bank].req_string = req.ip_string;
            }
        }
    }

    /*
     * Iterate over all requests in queue that are busy, and trigger the
     * corresponding FSMs
     */
    for (auto &req: req_queue) {
        if (req.busy == true) {
            /* Ensure we don't trigger the same FSM twice */
            if (bank[req.bank_group][req.bank].state == ACTIVATED ||
                bank[req.bank_group][req.bank].state == PRECHARGED) {
                if (req.row == bank[req.bank_group][req.bank].active_row) {
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
                } else {
                    /* Page Miss */
                    bank[req.bank_group][req.bank].state = PRECHARGE;
                }
            }
        }
    }
}
#else
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
#endif

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
            #ifdef RESCHEDULE_DEBUG
            std::cout << bank[bg][b].req_string << "\t\t - ";
            #endif
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
        } else if ((LAST_ACTIVATED_BANK_GRP != bg) && (time_since_bank_grp_ACT[LAST_ACTIVATED_BANK_GRP] < tRRD_S)) {
            break;
        }

        if (!bus_busy) {
            #ifdef RESCHEDULE_DEBUG
            std::cout << bank[bg][b].req_string << "\t\t - ";
            #endif
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
            if ((LAST_READ_BANK_GRP == bg) && (time_since_bank_grp_RD[bg] < tCCD_L)) {
                break;
            } else if ((LAST_READ_BANK_GRP != bg) && (time_since_bank_grp_RD[LAST_READ_BANK_GRP] < tCCD_S)) {
                break;
            }
        } else if (LAST_COMMAND == WR) {
            /* Last command was a WRITE */
            /* Check for tWTR_L / tWTR_S (Even lesser sanity...?) */
            if ((LAST_WRITTEN_BANK_GRP == bg) && (time_since_bank_grp_WR[bg] <= (tCWD + tBURST-1 + tWTR_L))) {
                break;
            } else if ((LAST_WRITTEN_BANK_GRP != bg) && (time_since_bank_grp_WR[LAST_WRITTEN_BANK_GRP] <= (tCWD + tBURST-1 + tWTR_S))) {
                break;
            }
        }

        if (!bus_busy) {
            /* Issue the read and set wait timings */
            #ifdef RESCHEDULE_DEBUG
            std::cout << bank[bg][b].req_string << "\t\t - ";
            #endif
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

            queue_remove(db.address);
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
            if ((LAST_WRITTEN_BANK_GRP == bg) && (time_since_bank_grp_WR[bg] < tCCD_L)) {
                break;
            } else if ((LAST_WRITTEN_BANK_GRP != bg) && (time_since_bank_grp_WR[LAST_WRITTEN_BANK_GRP] < tCCD_S)) {
                break;
            }
        }

        if (!bus_busy) {
            #ifdef RESCHEDULE_DEBUG
            std::cout << bank[bg][b].req_string << "\t\t - ";
            #endif
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

            queue_remove(db.address);
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
            db.busy = false;
            db.req_column = -1;
            db.req_row = -1;
        }
        break;

    default:
        break;
    }
}

/* DRAM operation */
void DRAM::do_ram_things()
{
    #ifdef RESCHEDULING
    /* Avoid starving requests */
    feed_starving_requests();
    #endif

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

    /* Increment time in queue */
    for (auto &req: req_queue)
        req.q_time += clock_diff;

    /* Unmark bus busy if enough clock cycles have passed */
    if (clock_diff >= bus_busy_timer) {
        bus_busy_timer = 0;
        bus_busy = false;
    }

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
    
    op << std::dec << cpu_clock_tick << "\t" << "ACT\t" << std::hex << std::uppercase << bank_group << "\t" << bank << "\t" << row << std::endl;
    
    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::precharge(uint64_t bank_group, uint64_t bank)
{
    std::stringstream op;

    op << std::dec << cpu_clock_tick << "\t" << "PRE\t" << std::hex << std::uppercase << bank_group << "\t" << bank << std::endl;
    
    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::read(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::stringstream op;

    op << std::dec << cpu_clock_tick << "\t" << "RD\t" << std::hex << std::uppercase << bank_group << "\t" << bank << "\t" << column << std::endl;

    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::write(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::stringstream op;
    
    op << std::dec << cpu_clock_tick << "\t" << "WR\t" << std::hex << std::uppercase << bank_group << "\t" << bank << "\t" << column << std::endl;

    std::cout << op.str();
    dram_cmd_file << op.str();
}

/* Rescheduling Functions */

/* Is requested row active? */
bool DRAM::is_req_page_active(request req)
{
    if (bank[req.bank_group][req.bank].active_row == req.row) {
        #ifdef RESCHEDULE_DEBUG
        std::cout << "Page active for request - BG: " << req.bank_group << "\tB: " << req.bank << "\tRow: " << req.row << std::endl;
        #endif
        return true;
    }
    
    return false;
}

/* Get the free location for a request in queue */
int8_t DRAM::queue_get_free_location()
{
    for (size_t i = 0; i < req_queue.size(); i++) {
        if (req_queue[i].busy == true) {
            // Go to the next request if element is busy
            continue;
        } else {
            // Just add it to the next "free" location
            // Ideally I think I want to maintain the order of requests
            // to the same bank group, bank, row .. But a little bit
            // complicated to implement
            return i;
        }
    }

    return 0;
}

/* Make sure requests aren't starving for too long */
void DRAM::feed_starving_requests()
{
    for (size_t i = 0; i < req_queue.size(); i++) {
        if ((req_queue[i].q_time >= STARVATION_THRESHOLD) && (req_queue[i].busy == false)) {

            /* Move starving request to next non-busy position in queue */
            request starved_req = req_queue[i];
            req_queue.erase(req_queue.begin()+i);

            int8_t pos = queue_get_free_location();
            req_queue.insert(req_queue.begin()+pos, starved_req);
        }
    }
}