# GM Binary format documentation V1.0

```cpp
struct Source_File {
    u4 magic_number = 0xACABACAB;
    u2 major_version;
    u2 minor_version;
    u8 data_section_address;
    u8 code_section_address;
};
```

## Data section format

```cpp
struct Data {
    u2 size;
    u1 data[size];
};

Data data_section[...] = {};
```

## Code section rules

```cpp
// argument types
using DataSectionAddress = u8;  // 00
using Register = u1;            // 01
using Literal = u8;             // 02

struct Instruction_No_Args {
    u2 instruction_num;
};

struct Instruction_One_Args {
    u2 instruction_num;
    u1 arg[sizeof(typeof(arg))];
};

struct Instruction_Two_Args {
    u2 instruction_num;
    u1 arg1[sizeof(typeof(arg1))];
    u1 arg2[sizeof(typeof(arg2))];
}
```
