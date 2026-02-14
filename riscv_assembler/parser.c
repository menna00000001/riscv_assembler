// parser.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "parser.h"
#include "riscv_instructions.h"
#include "instruction_args.h"

// Remove the extern declaration of NUM_RV32I_INSTRUCTIONS if it exists
// and use num_rv32i_instructions instead

static instr_def_t *find_instruction(const char *mnemonic, isa_extension_t isa) {
    extern instr_def_t rv32i_instructions[];
    extern size_t num_rv32i_instructions;  // Change from NUM_RV32I_INSTRUCTIONS

    for (size_t i = 0; i < num_rv32i_instructions; i++) {  // Use num_rv32i_instructions
        if (strcmp(rv32i_instructions[i].mnemonic, mnemonic) == 0 &&
            rv32i_instructions[i].isa_ext == isa) {
            return &rv32i_instructions[i];
        }
    }
    return NULL;
}

int parse_instruction(const char *line, parsed_instruction_t *parsed) {
    char mnemonic[16];
    char operands[256];

    // Skip leading whitespace
    while (isspace(*line)) line++;

    // Extract mnemonic
    int i = 0;
    while (*line && !isspace(*line) && i < 15) {
        mnemonic[i++] = *line++;
    }
    mnemonic[i] = '\0';

    // Skip to operands
    while (isspace(*line)) line++;

    strncpy(operands, line, 255);
    operands[255] = '\0';

    // Look up instruction in table
    parsed->def = find_instruction(mnemonic, ISA_RV32I);
    if (!parsed->def) {
        return 0; // Instruction not found
    }

    // Parse operands if parser function exists
    if (parsed->def->parser) {
    return parsed->def->parser(parsed->def, operands, &parsed->args);
}
    return 1;
}
