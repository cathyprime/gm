#pragma once
#include <cstdint>

using std::uint64_t;

enum Register : uint8_t {
    RAX, RBX, RCX, RSP, RBP, RDI, RSI, RDX,
    EAX, EBX, ECX, ESP, EBP, EDI, ESI, EDX,
    AX, BX, CX, SP, BP, DI, SI, DX,
    AH, BH, CH, DH = 31,
    AL, BL, CL, SPL, BPL, DIL, SIL, DL, IP
};

struct Register_Value {
    union {
        uint64_t as_value;
        Register as_register;
    };
    bool is_register;

    Register_Value(Register r)
        : as_register(r)
        , is_register(true){}

    Register_Value(uint64_t v)
        : as_value(v)
        , is_register(false){}

    operator bool()
    {
        return is_register;
    }
};

class CPU {
    uint64_t registers[8];
    uint64_t instruction;
    class Register_Proxy {
        uint64_t &reg_data;
        uint64_t mask;
        uint64_t shift;

      public:
        Register_Proxy(uint64_t &data, uint64_t mask = UINT64_MAX, uint64_t shift = 0);
        operator uint64_t() const;
        void operator++(int);
        void operator+=(uint64_t value);
        void operator-=(uint64_t value);
        void operator*=(uint64_t value);
        void operator/=(uint64_t value);
        void operator=(uint64_t value);
        void operator=(Register_Proxy &other);
    };

  public:
    Register_Proxy operator[](Register reg);
    uint64_t operator[](Register_Value &rg);
};
