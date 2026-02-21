#ifndef RISCV_INSTRUCTIONS_H_INCLUDED
#define RISCV_INSTRUCTIONS_H_INCLUDED

#include "instruction_defs.h"
#include "instruction_args.h"

// Declare the instruction table and its size
// Base ISA
extern instr_def_t rv32i_instructions[];
extern size_t num_rv32i_instructions;

extern instr_def_t rv64i_instructions[];
extern size_t num_rv64i_instructions;

// Zicsr extension
extern instr_def_t zicsr_instructions[];
extern size_t num_zicsr_instructions;

#endif // RISCV_INSTRUCTIONS_H_INCLUDED
