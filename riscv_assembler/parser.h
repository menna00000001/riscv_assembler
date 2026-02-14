#ifndef PARSER_H
#define PARSER_H

#include "instruction_defs.h"
#include "instruction_args.h"

typedef enum {
    INST_R,
    INST_I,
    INST_S,
    INST_B,
    INST_U,
    INST_J
} inst_type_t;

typedef struct {
    inst_type_t type;
    int rd, rs1, rs2;
    int imm;
    int shamt;
} instruction_t;

typedef struct {
    instr_def_t *def;    // Pointer to instruction definition
    instr_args_t args;   // Parsed arguments
} parsed_instruction_t;

int parse_line(const char *line, instruction_t *inst);

#endif // PARSER_H
