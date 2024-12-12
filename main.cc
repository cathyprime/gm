#include <iostream>
#include <climits>
#include <bitset>

#include "registers.hh"

int main()
{
    CPU cpu = {};
    cpu[RDX] = ULONG_MAX;
    std::cout << std::bitset<64>(cpu[DL]) << " -> " << cpu[DL] << std::endl;
    std::cout << std::bitset<64>(cpu[DH] << 8) << " -> " << (cpu[DH] << 8) << std::endl;
    std::cout << std::bitset<64>(cpu[DX]) << " -> " << cpu[DX] << std::endl;
    std::cout << std::bitset<64>(cpu[EDX]) << " -> " << cpu[EDX] << std::endl;
    std::cout << std::bitset<64>(cpu[RDX]) << " -> " << cpu[RDX] << std::endl;
}
