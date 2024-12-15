#include <iostream>
#include <climits>
#include <bitset>

#include "registers.hh"

int main()
{
    CPU cpu = {};
    cpu[RAX] = 10; // RAX: 10
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    cpu[RAX] -= 5; // RAX: 5
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    cpu[RAX] += 7; // RAX: 12
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    cpu[RAX] *= 2; // RAX: 24
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    cpu[RBX] = cpu[RAX]; // RBX: 24
    std::cout << "RBX -> " << cpu[RBX] << std::endl;

    cpu[RAX] += cpu[RBX]; // RAX: 48; RBX: 24
    std::cout << "RAX -> " << cpu[RAX] << std::endl;
    std::cout << "RBX -> " << cpu[RBX] << std::endl;
}
