#include <iostream>
#include <fstream>
#include <string>

#include "commondefs.h"
#include "dram.h"
#include "fileparse.h"

// CPU clock ticker
uint64_t cpu_clock_tick = 0;

int main(int argc, char *argv[])
{
    // Input Trace File
    std::string ip_trace_name;
    std::ifstream *ip_trace_fstream;
    std::string ip_string;
    bool ip_file_read_done {false};
    // Start off as request processed so
    // we can take in a first request
    bool req_fetched {true};

    // Check if input trace file is provided
    // First argument is always executable name
    if (argc == 1) {
        std::cerr << "No input trace file provided!" << std::endl;
        return 1;
    }

    // Read the input file name
    ip_trace_name = argv[1];
    std::cout << "Input file: " << ip_trace_name << std::endl;

    // Check if the file opens
    ip_trace_fstream = new std::ifstream(ip_trace_name);
    if (!ip_trace_fstream->is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    // Instantiate DRAM Controller
    DRAM dram_controller;
    request req; 
    
    // Start DRAM Controller loop
    while (true) {
        // Increment CPU Clock
        cpu_clock_tick++;

        // Check if we can read in a request
        if ((!ip_file_read_done) && (dram_controller.is_queue_full() == false)) {
            // Is previous request processed?
            // Read in a new one if it is
            if (req_fetched == true) {
                if (std::getline(*ip_trace_fstream, ip_string)) {
                    req = read_file(ip_string);
                    req_fetched = false;
                } else {
                    ip_file_read_done = true;
                }
            }
            // Add to the queue if the current CPU clock
            // is more than the requested clock tick
            if (cpu_clock_tick >= req.cpu_req_time) {
                // If queue is empty, we can "jump" time
                if (dram_controller.is_queue_empty()) {
                    std::cout << "Jumping to clock tick: " << req.cpu_req_time << std::endl;
                    cpu_clock_tick = req.cpu_req_time;
                }
                dram_controller.queue_add(req);
                req_fetched = true;
            }
        } else {
            // If queue is empty and file is done, it means that
            // our simulation is over
            if (dram_controller.is_queue_empty())
                break;
        }

        // Do RAM operations every n-th tick since DRAM clock is slower than CPU clock
        if (cpu_clock_tick % (CPU_CLK_FREQ / dram_controller.DRAM_CLK_FREQ) == 0)
            dram_controller.do_ram_things();
    }

    // Free the filestream heap
    delete ip_trace_fstream;

    return 0;
}