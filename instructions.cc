#include <cstdlib>
#include <iostream>
#include "instructions.hh"

void exec(CPU &cpu, Instruction inst)
{
    switch (inst.instruction_code) {
        case HLT:
            std::exit(0);
            break;
        case MOV:
            cpu[inst.first.as_register] = cpu[inst.second];
            break;
        case ADD:
            cpu[inst.first.as_register] += cpu[inst.second];
            break;
        case SUB:
            cpu[inst.first.as_register] -= cpu[inst.second];
            break;
        case MUL:
            cpu[inst.first.as_register] *= cpu[inst.second];
            break;
        case DIV:
            cpu[inst.first.as_register] /= cpu[inst.second];
            break;
        case PSH:
        case POP:
        case AND:
        case OR:
        case XOR:
        case NOT:
        case INT:
        case JEQ:
        case JNE:
        case JMP:
        case JGT:
        case JLT:
            std::cerr << "Unimplemented!" << std::endl;
            std::exit(420);
        default:
            std::exit(1);
    }
    cpu[IP]++;
}
