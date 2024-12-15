enum Instructions : uint8_t {
    HLT = 1, MOV, ADD,
    SUB, MUL, DIV,
    PSH, POP, AND,
    OR, XOR, NOT,
    INT, JEQ, JNE,
    JMP, JGT, JLT,
};
