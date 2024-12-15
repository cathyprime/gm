#include <cstdint>

#include "registers.hh"

enum Instruction_Code : std::uint64_t {
    HLT = 1, MOV, ADD,
    SUB, MUL, DIV,
    PSH, POP, AND,
    OR, XOR, NOT,
    INT, JEQ, JNE,
    JMP, JGT, JLT,
};

uint64_t operator+(CPU &cpu, Register_Value &rv);

struct Instruction {
    Instruction_Code instruction_code;
    Register_Value first;
    Register_Value second;
};

void exec(CPU &cpu, Instruction instruction);
