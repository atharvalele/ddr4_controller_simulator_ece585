# DDR4 Memory Controller Simulator using C++
### ECE585 Final Project - Group 9 ;)
#### Atharva Lele, Yashodhan Wagle, Supreet Gulavani, Ayush Srivastava
----------------------------------------------------------------------

### How to run:
- Assuming you have GCC and Make installed, `cd` to the project directory and run make
- There are a number of targets available
    
    `make debug` : First ready, first access with debug info

    `make release` : First ready, first access without debug info

    `make rescheduling_debug` :     Out-of-order with debug info

    `make rescheduling_release` :    Out-of-order without debug info

- You should now have the executable named `ece585_memory_controller` in the top directory.
- Run it as follows:

    ```$ ./ece585_memory_controller <input_trace_file> <output_dram_commands_file>```

- For example, if my input memory request trace is `ip_trace.txt` and I want output in the file named `output.txt`, I will run:

    ```$ ./ece585_memory_controller ip_trace.txt output.txt```

### Configuration Options
- Bank parallelism:
    - To enable, make sure the line `#define BANK_PARALLELISM` in `include/commondefs.h` is _uncommented_.
    - To disable, make sure the line is commented.
    - Note: You need to `make clean` and recompile the project for the effect to take place.
