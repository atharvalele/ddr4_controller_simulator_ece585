#include "fileparse.h"

//#define DRAM_FILE "commands.txt"

void read_file( int dummy_set, std::ifstream &cmd_file)
{
    size_t pos = 0;
    //std::ifstream cmd_file (DRAM_FILE); // Open file ** DEPRECATED **
    std::string cmd_string, delimiter = " ";
    std::string token[3];
    int i = 0;// j = 0;
    if (dummy_set) // Variable checks if queue is empty 
    {    
        std::getline(cmd_file, cmd_string); // Grabs the line from the input file
        std::cout << cmd_string << std::endl;
    }

    while ((pos = cmd_string.find(delimiter)) != std::string::npos) { // Seperate tokens
        token[i] = cmd_string.substr(0, pos); // Grab substring till first delimiter
        std::cout << token[i] << std::endl;   // Print token 
        cmd_string.erase(0, pos + delimiter.length()); // Erase string after extractions
        i++;
    }

}

/* ** DEPRECATED TEST FUNCTION **
int main()
{   
    std::ifstream cmd_file (DRAM_FILE);
    read_file(1, cmd_file);
    read_file(1, cmd_file);
}
*/