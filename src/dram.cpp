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

/* DRAM operation */
void DRAM::do_ram_things()
{
    /* Increment Clock */
    clock_tick++;

    /* Increment time in queue for all */
    for (auto &r: req_queue) {
        r.q_time++;
    }
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

void DRAM::dram_read(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::stringstream op;

    op << std::dec << cpu_clock_tick << "\t" << "RD\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << column << std::endl;

    std::cout << op.str();
    dram_cmd_file << op.str();
}

void DRAM::dram_write(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::stringstream op;
    
    op << std::dec << cpu_clock_tick << "\t" << "WR\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << column << std::endl;

    std::cout << op.str();
    dram_cmd_file << op.str();
}