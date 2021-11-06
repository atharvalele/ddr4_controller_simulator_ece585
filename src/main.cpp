#include <iostream>
#include <fstream>
#include <string>

#include "fileparse.h"

int main(int argc, char *argv[])
{
    // Input Trace File
    std::string ip_trace_name;
    std::ifstream *ip_trace_fstream;

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

    // Add loop here for RAM events
    read_file(*ip_trace_fstream);

    // Free the filestream heap
    delete ip_trace_fstream;

    return 0;
}