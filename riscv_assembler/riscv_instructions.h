#ifndef RISCV_INSTRUCTIONS_H_INCLUDED
#define RISCV_INSTRUCTIONS_H_INCLUDED

#include "instruction_defs.h"
#include "instruction_args.h"

// Declare the instruction table and its size
extern instr_def_t rv32i_instructions[];
extern size_t num_rv32i_instructions;  // Use lowercase 'n' for consistency

extern instr_def_t rv64i_instructions[];
extern size_t num_rv64i_instructions;

#endif // RISCV_INSTRUCTIONS_H_INCLUDED
