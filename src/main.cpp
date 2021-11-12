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

    // Print starting message
    std::cout << "\n -- Starting Simulation -- \n" << std::endl;

    // Start DRAM Controller loop
    while (true) {
        // Check if we can read in a request
        if ((ip_trace_fstream->is_open()) && (dram_controller.is_queue_full() == false) && req_fetched) {
            // Read a new line
            ip_string = "";
            while (ip_string.empty() && ip_trace_fstream->is_open()) {
                std::getline(*ip_trace_fstream, ip_string);
                // File over?
                if (ip_trace_fstream->eof()) {
                    ip_trace_fstream->close();
                    break;
                }
            }
            req = read_file(ip_string);
            req_fetched = false;
        }

        // Is it time to add the request to the queue?
        if ((!dram_controller.is_queue_full()) && !req_fetched) {
            // Is the queue empty? If so just advance CPU time
            if ((dram_controller.is_queue_empty()) && (cpu_clock_tick < req.cpu_req_time)) {
                #ifdef DEBUG
                std::cout << "Advacing CPU time from " << std::dec << cpu_clock_tick << ": " << req.cpu_req_time << std::endl;
                #endif
                cpu_clock_tick = req.cpu_req_time;
            }
            
            if (cpu_clock_tick >= req.cpu_req_time) {
                dram_controller.queue_add(req);
                req_fetched = true;
            }
        }

        // Increment CPU Clock
        cpu_clock_tick++;

        // Do RAM operations every n-th tick since DRAM clock is slower than CPU clock
        if (cpu_clock_tick % (CPU_CLK_FREQ / dram_controller.DRAM_CLK_FREQ) == 0)
            dram_controller.do_ram_things();

        // Break if queue is empty and file is closed
        if (!(ip_trace_fstream->is_open()) && dram_controller.is_queue_empty() && req_fetched) {
            std::cout << "\n -- Simulation Over! -- \n" << std::endl;
            break;
        }
    }

    // Free the filestream heap
    delete ip_trace_fstream;

    return 0;
}