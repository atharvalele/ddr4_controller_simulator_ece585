#include "dram.h"

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

    switch(state)
    {
        case IDLE: 
            if(!is_queue_empty())
                state = PRE;
        break;
        case PRE:
            precharge(req_queue.front().bank_group, req_queue.front().bank);
            state = ACT;
        break;
        case PRE_WAIT:
        break;
        case ACT:
            activate(req_queue.front().bank_group, req_queue.front().bank, req_queue.front().row);
            if(req_queue.front().req_type == 0 || req_queue.front().req_type == 2)
                state = READ;
            else if (req_queue.front().req_type == 1)
                state = WRITE;
        break;
        case ACT_WAIT:  
        break;
        case READ:
            dram_read(req_queue.front().bank_group, req_queue.front().bank, (req_queue.front().high_col << 3));
            state = IDLE;
            queue_remove();
        break;
        case WRITE:
            dram_write(req_queue.front().bank_group, req_queue.front().bank, (req_queue.front().high_col << 3));
            state = IDLE;
            queue_remove();
        break;
    }
    
}

void DRAM::activate(uint64_t bank_group, uint64_t bank, uint64_t row)
{
    std::cout << std::dec <<cpu_clock_tick << "\t" << "ACT\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << row << std::endl;
}

void DRAM::precharge(uint64_t bank_group, uint64_t bank)
{
    std::cout << std::dec << cpu_clock_tick << "\t" << "PRE\t0x" << std::hex << bank_group << "\t0x" << bank << std::endl;
}

void DRAM::dram_read(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::cout << std::dec << cpu_clock_tick << "\t" << "RD\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << column << std::endl;
}

void DRAM::dram_write(uint64_t bank_group, uint64_t bank, uint64_t column)
{
    std::cout << std::dec << cpu_clock_tick << "\t" << "WR\t0x" << std::hex << bank_group << "\t0x" << bank << "\t0x" << column << std::endl;
}