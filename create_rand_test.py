import os
import random

# Pad hex string with zeroes
def padhexa(s):
    return '0x' + s[2:].zfill(9).upper()

filename = input('Enter filename: ')
num_cases = int(input('Enter number of test cases: '))

cpu_tick = 0

data_addresses = []
instr_addresses = []

# Create some random addresses for data and instructiond
for i in range(0, 20):
    data_addr = random.randint(0, 4563402744)
    # Mask 3 LSB bits to be 8-byte aligned
    data_addr = data_addr & ~0x7

    instr_addr = random.randint(0, 4563402744)
    # Mask 3 LSB bits to be 8-byte aligned
    instr_addr = instr_addr & ~0x7

    data_addresses.append(hex(data_addr))
    instr_addresses.append(hex(instr_addr))
    


with open(filename, 'w') as f:
    for i in range(0, num_cases):
        # Write clock tick
        f.write(str(cpu_tick) + '\t')
        
        # Write operation type
        op = random.randint(0, 2)
        f.write(str(op) + '\t')

        # Write memory address
        if op == 2:
            hexstr = padhexa(str(random.choice(instr_addresses)))
            f.write(hexstr)
        else:
            hexstr = padhexa(str(random.choice(data_addresses)))
            f.write(hexstr)

        f.write('\n')

        # Increase ticks
        cpu_tick = cpu_tick + 1