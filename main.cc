#include <iostream>

#include "registers.hh"
#include "instructions.hh"

int main()
{
    CPU cpu = {};
    exec(cpu, Instruction {
        .instruction_code = MOV,
        .first = RAX,
        .second = 10
    });
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    exec(cpu, Instruction {
        .instruction_code = SUB,
        .first = RAX,
        .second = 5
    });
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    exec(cpu, Instruction {
        .instruction_code = ADD,
        .first = RAX,
        .second = 7
    });
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    exec(cpu, Instruction {
        .instruction_code = MUL,
        .first = RAX,
        .second = 2
    });
    std::cout << "RAX -> " << cpu[RAX] << std::endl;

    exec(cpu, Instruction {
        .instruction_code = MOV,
        .first = RBX,
        .second = RAX
    });
    std::cout << "RBX -> " << cpu[RBX] << std::endl;

    exec(cpu, Instruction {
        .instruction_code = ADD,
        .first = RAX,
        .second = RBX
    });
    std::cout << "RAX -> " << cpu[RAX] << std::endl;
    std::cout << "RBX -> " << cpu[RBX] << std::endl;
}
