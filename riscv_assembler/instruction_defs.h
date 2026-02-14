// instruction_defs.h
#ifndef INSTRUCTION_DEFS_H
#define INSTRUCTION_DEFS_H

#include <stdint.h>

typedef enum {
    TYPE_R,
    TYPE_I,
    TYPE_I7,
    TYPE_S,
    TYPE_B,
    TYPE_U,
    TYPE_J,
    TYPE_R4,   // For F/D extension
    TYPE_C     // For compressed extension
} instr_format_t;

typedef enum {
    ISA_RV32I,
    ISA_RV64I,
    ISA_RV32M,  // Multiplication
    ISA_RV32F,  // Float
    ISA_RV32D,  // Double
    ISA_RV32C,  // Compressed
    ISA_RV32V   // Vector
} isa_extension_t;

typedef struct instr_def_t instr_def_t; // forward declaration for self-pointer

//typedef uint32_t (*encoder_fn_t)(const instr_def_t *def, const void *args);
//typedef int      (*parser_fn_t)(const instr_def_t *def, const char *line, void *args);

struct instr_def_t {
    const char *mnemonic;      // Instruction name
    instr_format_t format;     // Instruction format
    uint8_t opcode;            // Base opcode
    uint8_t funct3;            // funct3 field (if applicable)
    uint8_t funct7;            // funct7 field (if applicable)
    uint8_t funct2;            // For R4 format
    isa_extension_t isa_ext;   // Which ISA extension this belongs to
    uint32_t (*encoder)(const instr_def_t *, const void *);
    int      (*parser)(const instr_def_t *, const char *, void *);
};

#endif
