#include <iostream>

#include "registers.hh"

static uint64_t registers[8];

CPU::Register_Proxy::Register_Proxy(uint64_t &data, uint64_t mask, uint64_t shift)
    : reg_data(data)
    , mask(mask)
    , shift(shift)
{ }

CPU::Register_Proxy::operator uint64_t() const
{
    return (reg_data >> shift) & mask;
}

void CPU::Register_Proxy::operator=(uint64_t value)
{
    reg_data = (reg_data & ~(mask << shift)) | ((value & mask) << shift);
}

void CPU::Register_Proxy::operator=(Register_Proxy &r1)
{
    this->reg_data = (uint64_t)r1;
}

CPU::Register_Proxy CPU::operator[](Register reg)
{
    if (reg >= RAX && reg <= RDX)
        return Register_Proxy(registers[reg]);

    if (reg >= EAX && reg <= EDX)
        return Register_Proxy(registers[reg % 8], 0xFFFFFFFF);

    if (reg >= AX && reg <= DX)
        return Register_Proxy(registers[reg % 8], 0xFFFF);

    if (reg >= AH && reg <= CH || reg == DH)
        return Register_Proxy(registers[reg % 8], 0xFF, 8);

    if (reg >= AL && reg <= DL)
        return Register_Proxy(registers[reg % 8], 0xFF);

    std::cerr << "FUCK YOU\n";
    std::exit(69);
}
