#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "commondefs.h"
#include "fileparse.h"
#include "request.h"

/*
 * Parses the input trace string
 * Prints to debug console if built in debug mode
 */
void read_file(std::string ip_string, request& req)
{
    uint32_t substr_begin = 0, substr_end, pos = 0;

    std::string tokens[3];
    uint8_t token_count = 0;

    bool token_valid {false};

    // Traverse the line and get 3 tokens
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

            token_count++;
            token_valid = true;
        }
        else {
            break;
        }

        // Invalid input checking
        if (token_count > 3) {
            std::cerr << "Invalid input! - " << ip_string << std::endl;
            exit(1);
        }
    }

    // Get the data into variables
    if (token_valid == true) {
        req.cpu_req_time = std::stoull(tokens[0], nullptr);
        req.q_time = 0;
        req.req_type = (REQ_OP)std::stoi(tokens[1], nullptr);
        req.address = std::stoull(tokens[2], nullptr, 16);
        req.row = (req.address & 0x1FFFC0000) >> 18;
        req.high_col=((req.address & 0x3FC00) >> 7);
        req.bank = (req.address & 0x300) >> 8;
        req.bank_group = (req.address & 0xC0) >> 6;
        req.burst_index = ((req.address & 0x38) >> 3);
        req.col = req.high_col | req.burst_index;
        req.valid = true;
    } else {
        req.valid = false;
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

    if (token_valid)
        std::cout << "CPU Req Time: " << tokens[0] << "\tType: " << op << "\tAddress: " << tokens[2] << std::endl;
    #endif
}