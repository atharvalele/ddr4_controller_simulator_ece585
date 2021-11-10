#include "dram.h"

/* Add element to DRAM queue */
void DRAM::queue_add(request req)
{
    if (req_queue.size() < QUEUE_SIZE) {
        req_queue.push_back(req);
        std::cout << req << std::endl;
    } else {
        std::cout << "Queue Full" << std::endl;
    }
}

/* Remove element from the queue */
void DRAM::queue_remove()
{
    std::cout << "Removed from queue!\n";
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
