#include <iostream>
#include <climits>
#include <bitset>

#include "registers.hh"
#include "instructions.hh"

using namespace instructions;

int main()
{
    CPU cpu = {};
    mov(cpu[RAX], 10); // RAX: 10
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    sub(cpu[RAX], 5); // RAX: 5
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    add(cpu[RAX], 7); // RAX: 12
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    mul(cpu[RAX], 2); // RAX: 24
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    mov(cpu[RBX], cpu[RAX]); // RBX: 24
    std::cout << "RBX -> " << cpu[RBX] << std::endl;

    add(cpu[RAX], cpu[RBX]); // RAX: 48; RBX: 24
    std::cout << "RAX -> " << cpu[RAX] << std::endl;
    std::cout << "RBX -> " << cpu[RBX] << std::endl;
}
