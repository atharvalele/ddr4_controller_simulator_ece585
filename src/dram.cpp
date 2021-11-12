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
        std::cout << "Removed from Queue: CPU Clock: " << std::dec << cpu_clock_tick << " - " << req_queue.front();
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

    /* Remove once an item is present for a 100 CPU clocks */
    for (auto &r: req_queue) {
        if (r.q_time >= 100 / (CPU_CLK_FREQ / DRAM_CLK_FREQ)) {
            queue_remove();
            // Break, so we effectively "clock out" the requests
            break;
        }
    }
}