
# RISC-V Assembler

## 🧠 Project Description
This is a simple **RISC-V assembler** written in C.  
It reads assembly code (`.s` files) and outputs machine code in **hex format**.  
Supports word and byte instructions. Designed for learning and experimentation with the RISC-V ISA.

## 🗂 Folder Structure / Files


riscv_assembler/
├─ main.c            # Entry point of the assembler

├─ parser.c          # Handles parsing of assembly instructions

├─ encoder.c         # Converts parsed instructions to machine code

├─ riscv_instructions.h  # Defines supported RISC-V instructions

├─ parser.h          # Header for parser functions

├─ encoder.h         # Header for encoder functions

├─ instruction_args.h  # Instruction arguments definitions

- **main.c** – orchestrates reading, parsing, and encoding.  
- **parser.c / parser.h** – reads `.s` files and breaks instructions into components.  
- **encoder.c / encoder.h** – generates machine code in hexadecimal format.  
- **riscv_instructions.h / instruction_args.h** – defines supported instructions and their argument formats.  

## GCC Installation (Windows)  
This project requires **GCC** to compile the assembler.  

### Recommended Method: MSYS2  
1. Download MSYS2 from: [https://www.msys2.org](https://www.msys2.org)  
2. Install MSYS2 (default path: `C:\msys64`)  
3. Open **MSYS2 MinGW64** terminal (not MSYS)  
4. Install GCC:   
```bash    
pacman -S --needed base-devel mingw-w64-x86_64-gcc
````

5. Add GCC to PATH:

```text
C:\msys64\mingw64\bin
```

6. Verify installation:

```bash
gcc --version
```

## How to run the assembler

Compile the project:

```powershell
gcc main.c parser.c encoder.c -o assembler
```

Run the assembler for **word instructions**:

```powershell
.\assembler.exe input.s output.hex word
```

Run the assembler for **byte instructions**:

```powershell
.\assembler.exe input.s output.hex byte
```
