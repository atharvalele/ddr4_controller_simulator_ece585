# DDR4 Memory Controller Simulator using C++
### ECE585 Final Project - Group 9 ;)
#### Atharva Lele, Yashodhan Wagle, Supreet Gulavani, Ayush Srivastava
----------------------------------------------------------------------

### How to run:
- Assuming you have GCC and Make installed, `cd` to the project directory and run
    
    `make debug` : Build with debug info printed to console

    `make release` : Build without debug info printed to the console

- You should now have the executable named `ece585_memory_controller` in the top directory.
- Run it as follows:

    ```$ ./ece585_memory_controller <input_trace_file> <output_dram_commands_file>```

- For example, if my input memory request trace is `ip_trace.txt` and I want output in the file named `output.txt`, I will run:

    ```$ ./ece585_memory_controller ip_trace.txt output.txt```
