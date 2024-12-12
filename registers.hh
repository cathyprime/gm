#include <cstdint>

using std::uint64_t;

enum Register {
    RAX, RBX, RCX, RSP, RBP, RDI, RSI, RDX,
    EAX, EBX, ECX, ESP, EBP, EDI, ESI, EDX,
    AX, BX, CX, SP, BP, DI, SI, DX,
    AH, BH, CH, DH = 31,
    AL, BL, CL, SPL, BPL, DIL, SIL, DL,
};

class CPU {
    class Register_Proxy {
        uint64_t &reg_data;
        uint64_t mask;
        uint64_t shift;

      public:
        Register_Proxy(uint64_t &data, uint64_t mask = UINT64_MAX, uint64_t shift = 0);
        operator uint64_t() const;
        void operator=(uint64_t value);
    };

  public:
    Register_Proxy operator[](Register reg);
};
