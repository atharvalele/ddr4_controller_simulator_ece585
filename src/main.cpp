#include <iostream>
#include <fstream>
#include <string>

#include "dram.h"
#include "fileparse.h"

int main(int argc, char *argv[])
{
    // Input Trace File
    std::string ip_trace_name;
    std::ifstream *ip_trace_fstream;
    std::string ip_string;

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
        // Check if we can read in a request
        if (dram_controller.is_queue_full() == false) {
            std::getline(*ip_trace_fstream, ip_string);
            req = read_file(ip_string);
            dram_controller.queue_add(req);
        } else {
            std::cout << "Queue FULL!" << std::endl;
            break;
        }
    }

    // Free the filestream heap
    delete ip_trace_fstream;

    return 0;
}