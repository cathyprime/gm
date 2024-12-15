#include <iostream>

#include "registers.hh"

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

void CPU::Register_Proxy::operator=(Register_Proxy &other)
{
    *this = static_cast<uint64_t>(other);
}

CPU::Register_Proxy CPU::operator[](Register reg)
{
    if (reg == IP)
        return Register_Proxy(instruction);

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

    std::cerr << "FUCK YOU " << reg << " is not a register!!" << '\n';
    std::exit(69);
}

uint64_t CPU::operator[](Register_Value &rv)
{
    if (rv) {
        return (*this)[rv.as_register];
    } else {
        return rv.as_value;
    }
}

void CPU::Register_Proxy::operator+=(uint64_t value)
{
    reg_data = reg_data + static_cast<uint64_t>(value);
}

void CPU::Register_Proxy::operator-=(uint64_t value)
{
    reg_data = reg_data - static_cast<uint64_t>(value);
}

void CPU::Register_Proxy::operator*=(uint64_t value)
{
    reg_data = reg_data * static_cast<uint64_t>(value);
}

void CPU::Register_Proxy::operator/=(uint64_t value)
{
    reg_data = reg_data / static_cast<uint64_t>(value);
}

void CPU::Register_Proxy::operator++(int)
{
    reg_data = reg_data + 1;
}
