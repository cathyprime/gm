#pragma once
#include <cstddef>

#include "registers.hh"

namespace instructions {

using Value = uint64_t;

void mov(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2);
void mov(CPU::Register_Proxy &&r1, Value v);

void add(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2);
void add(CPU::Register_Proxy &&r1, Value v);

void sub(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2);
void sub(CPU::Register_Proxy &&r1, Value v);

void mul(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2);
void mul(CPU::Register_Proxy &&r1, Value v);

void div(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2);
void div(CPU::Register_Proxy &&r1, Value v);

}
