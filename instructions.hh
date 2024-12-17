#include <cstdint>

#include "registers.hh"

enum Instruction_Code : std::uint16_t {
    MOV = 1, ADD,
    SUB, MUL, DIV,
    PSH, POP, AND,
    OR, XOR, NOT,
    INT, JEQ, JNE,
    JMP, JGT, JLT,
    HLT = 0xF4,
};

struct Instruction {
    Instruction_Code instruction_code;
    Register_Value first;
    Register_Value second;
};

void exec(CPU &cpu, Instruction instruction);
void dispatch_interrupt(CPU &cpu, int interrupt);
