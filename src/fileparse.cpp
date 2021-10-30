#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "fileparse.h"

/*
 * Parses the input trace file
 * Prints to debug console if built in debug mode
 */
void read_file(std::ifstream &cmd_file)
{
    // uint64_t cpu_req_clk;
    // uint8_t mem_op;
    // uint64_t mem_addr;

    uint32_t substr_begin = 0, substr_end, pos = 0;

    std::string ip_string;
    std::string tokens[3];
    uint8_t token_count = 0;

    // Traverse the line and get 3 tokens
    while (std::getline(cmd_file, ip_string)) {
        pos = 0;
        token_count = 0;
        
        while (pos < ip_string.length()-1) {
            // Find the first non-whitespace character
            while (isspace(ip_string[pos])) {
                pos++;
            }
            substr_begin = pos;

            // Find the next whitespace character
            while ((!isspace(ip_string[pos])) && (pos < ip_string.length())) {
                pos++;
            }
            substr_end = pos;
            
            // Data is between the two positions we found out
            if (substr_begin != substr_end) {
                tokens[token_count] = ip_string.substr(substr_begin, substr_end - substr_begin);
                //std::cout << "Begin: " << substr_begin << " End: " << substr_end << std::endl;
                //std::cout << tokens[token_count] << std::endl;
            }
            else {
                std::cout << "File over!";
                exit(1);
            }

            // Invalid input checking
            token_count++;
            if (token_count > 3) {
                std::cerr << "Invalid input! - " << ip_string << std::endl;
                exit(1);
            }
        }

        // Print out to debug
        #ifdef DEBUG
        std::string op;
        if (tokens[1] == "0")
            op = "READ";
        else if (tokens[1] == "1")
            op = "WRITE";
        else if (tokens[1] == "2")
            op = "FETCH";
        else
            op = "UNKNOWN";

        std::cout << "CPU Req Time: " << tokens[0] << "\tType: " << op << "\tAddress: " << tokens[2] << std::endl;
        #endif
    }
}