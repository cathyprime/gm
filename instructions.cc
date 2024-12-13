#include "instructions.hh"

namespace instructions {

void mov(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2)
{
    r1 = r2;
}

void mov(CPU::Register_Proxy &&r1, Value v)
{
    r1 = v;
}

void add(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2)
{
    r1 = r1 + r2;
}

void add(CPU::Register_Proxy &&r1, Value v)
{
    r1 = r1 + v;
}

void sub(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2)
{
    r1 = r1 - r2;
}

void sub(CPU::Register_Proxy &&r1, Value v)
{
    r1 = r1 - v;
}

void mul(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2)
{
    r1 = r1 * r2;
}

void mul(CPU::Register_Proxy &&r1, Value v)
{
    r1 = r1 * v;
}

void div(CPU::Register_Proxy &&r1, CPU::Register_Proxy &&r2)
{
    r1 = r1 / r2;
}

void div(CPU::Register_Proxy &&r1, Value v)
{
    r1 = r1 / v;
}

}
