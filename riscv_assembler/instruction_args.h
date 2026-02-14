// instruction_args.h
#ifndef INSTRUCTION_ARGS_H
#define INSTRUCTION_ARGS_H

typedef struct {
    int rd;
    int rs1;
    int rs2;
    int imm;
    int shamt;     // Shift amount for immediate shifts
} instr_args_t;

#endif
