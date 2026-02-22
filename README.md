# RISC-V Assembler

## 🧠 Project Description

This is a **modular RISC-V assembler** written in C.

* Reads RISC-V assembly files (`.s`) and outputs machine code in **hex format**.
* Supports **RV32I** and **RV64I** base ISA.
* Supports ZICSR system instructions (e.g., `ecall`, `ebreak`, `mret`, CSR access like `csrrw`, `csrrs`, `csrrc`, including both numeric CSR addresses and CSR symbolic names such as `mtvec`, `mepc`, `mcause`, `mtval`).
* Supports the RISC-V M extension (integer multiplication and division instructions).
* Supports **label resolution** for **B-type** (branches) and **J-type** (jumps) instructions.
* Supports both **word (32-bit)** and **byte (8-bit)** output modes.
* Designed as a **modular system**: parser, encoder, and instruction definitions are separate, making it easy to **extend to new ISAs or instructions**.

---

## 🗂 Folder Structure / Files

```
riscv_assembler/
├─ main.c                   # Entry point: orchestrates parsing, encoding, and output of machine code
├─ parser.c / parser.h      # Breaks instructions into components, resolves labels, and prepares arguments
├─ encoder.c / encoder.h    # Converts parsed instructions into binary machine code
├─ riscv_instructions.c / .h  # Contains definitions of supported RISC-V instructions and associated encoders/parsers
├─ instruction_args.h       # Defines structures for instruction arguments (rd, rs1, rs2, imm, shamt, etc.)
├─ instruction_defs.h       # Defines instruction formats, ISA extensions, and instr_def_t:
│                             - instr_format_t: R/I/S/B/U/J/… formats
│                             - isa_extension_t: base and optional extensions (ZICSR, M, F, D, C, V)
│                             - instr_def_t: holds opcode, funct3/funct7/funct12, ISA, and pointers to encoder/parser functions
```

**Highlights:**

* `main.c` – manages reading input `.s` files, calling the parser and encoder, and writing `.hex` output.
* `parser.c / parser.h` – parses instruction lines, extracts mnemonics and operands, resolves labels.
* `encoder.c / encoder.h` – encodes instructions into 32-bit machine code.
* `riscv_instructions.c / .h` – defines supported instructions, formats, and their parser/encoder functions.
* `instruction_args.h` – holds instruction argument structures (`rd`, `rs1`, `imm`, etc.).
* `instruction_defs.h` – contains all instruction metadata, including formats, ISA extensions, and pointers to parsing/encoding functions.

---

## ⚙ Features

| Feature                   | Details                                                                               |
| ------------------------- | ------------------------------------------------------------------------------------- |
| Supported ISAs            | RV32I, RV64I                                                                          |
| Instruction types         | R, I, I7, S, B, U, J                                                                  |
| M Extension               | Supports integer multiplication/division instructions (`mul`, `mulh`, `div`, `rem`, etc.) |
| CSR Addressing            | Supports both numeric CSR addresses (e.g., `0x305`) and symbolic CSR names (`mtvec`, `mepc`, etc.) |
| Endianness                | Outputs machine code in little-endian byte order (RISC-V standard) |
| Label support             | B-type (`beq`, `bne`, etc.) and J-type (`jal`) instructions                           |
| Output modes              | Word (32-bit) or Byte (8-bit) hex                                                     |
| Modular design            | Parser, encoder, instruction definitions are separate and extensible                  |
| Comments                  | Lines starting with `#` are ignored                                                   |

---

## 🏗 Modular Design

The assembler is structured so that you can:

* **Add new instructions**: just define their encoder, parser, and entry in the instruction table.
* **Add new ISA extensions**: RV32M, RV32F, RV64M, etc., without changing the main assembler loop.
* **Change output formats**: currently supports word and byte hex, can be extended for binary or other formats.

---

## 💻 GCC Installation (Windows)

This project requires **GCC** to compile the assembler.

### Recommended Method: MSYS2

1. Download MSYS2: [https://www.msys2.org](https://www.msys2.org)
2. Install MSYS2 (default path: `C:\msys64`)
3. Open **MSYS2 MinGW64** terminal (not MSYS)
4. Install GCC:

```bash
pacman -S --needed base-devel mingw-w64-x86_64-gcc
```

5. Add GCC to `PATH`:

```text
C:\msys64\mingw64\bin
```

6. Verify installation:

```bash
gcc --version
```

---

## 🏃 How to Run the Assembler

Compile the project:

```powershell
gcc main.c parser.c encoder.c riscv_instructions.c -o assembler
```

Run the assembler for **word output**:

```powershell
.\assembler.exe input.s output.hex word
```

Run the assembler for **byte output**:

```powershell
.\assembler.exe input.s output.hex byte
```

---

## 📝 Example Assembly (`input.s`)

```asm
# Example demonstrating labels, jumps, and backward offsets

    addi x1, x0, 5        # x1 = 5
    addi x2, x0, 10       # x2 = 10

loop_start:
    beq x1, x2, end       # Branch if x1 == x2
    addi x1, x1, 1        # Increment x1
    jal x0, loop_start    # Jump back to loop_start

end:
    addi x3, x0, 42       # End value
```

### Expected Word Output (`output.hex`)

```
00500093
00A00113
00208663
00108093
FF5FF06F
02A00193
```


---
