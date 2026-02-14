#include "instruction_args.h"
#include "instruction_defs.h"
#include "encoder.h"
#include "riscv_instructions.h"
#include <string.h>
#include <stdio.h>

// ==================== ENCODING FUNCTIONS ====================
static uint32_t encode_dispatch(const instr_def_t *def, const void *args) {
    const instr_args_t *a = (const instr_args_t *)args;

    switch (def->format) {
        case TYPE_R:
            return encode_R(def->funct7, a->rs2, a->rs1, def->funct3, a->rd, def->opcode);

        case TYPE_I:
            return encode_I(a->imm, a->rs1, def->funct3, a->rd, def->opcode);

        case TYPE_I7:
            return encode_I7(def->funct7, a->shamt, a->rs1, def->funct3, a->rd, def->opcode);

        case TYPE_S:
            return encode_S(a->imm, a->rs2, a->rs1, def->funct3, def->opcode);

        case TYPE_B:
            return encode_B(a->imm, a->rs2, a->rs1, def->funct3, def->opcode);

        case TYPE_U:
            return encode_U(a->imm, a->rd, def->opcode);

        case TYPE_J:
            return encode_J(a->imm, a->rd, def->opcode);

        default:
            return 0;
    }
}

// ==================== PARSING FUNCTIONS ====================
int (*parser)(const instr_def_t *, const char *, void *);
static int parse_dispatch(const instr_def_t *def, const char *line, void *args)
{
    instr_args_t *a = (instr_args_t *)args;

    switch (def->format) {

        case TYPE_R:
            return sscanf(line,"x%d, x%d, x%d", &a->rd, &a->rs1, &a->rs2);

        case TYPE_I:
            return sscanf(line,"x%d, x%d, %d", &a->rd, &a->rs1, &a->imm);

        case TYPE_I7:
            return sscanf(line,"x%d, x%d, %d", &a->rd, &a->rs1, &a->shamt);

        case TYPE_S:
            return sscanf(line,"x%d, %d(x%d)", &a->rs2, &a->imm, &a->rs1);

        case TYPE_B:
            return sscanf(line,"x%d, x%d, %d", &a->rs1, &a->rs2, &a->imm);

        case TYPE_U:
            return sscanf(line, "x%d, %d", &a->rd, &a->imm);

        case TYPE_J:
            return sscanf(line, "x%d, %d", &a->rd, &a->imm);

        default:
            return 0;
    }
}

// ==================== INSTRUCTION TABLE ====================

// RV32I Base Instructions (Complete set)
instr_def_t rv32i_instructions[] = {
     /* ---------------- R-Type ---------------- */
    {"add",  TYPE_R, 0x33, 0b000, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"sub",  TYPE_R, 0x33, 0b000, 0x20, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"sll",  TYPE_R, 0x33, 0b001, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // shift left logical
    {"slt",  TYPE_R, 0x33, 0b010, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // set less than
    {"sltu",  TYPE_R, 0x33, 0b011, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // set less than unsigned
    {"xor",  TYPE_R, 0x33, 0b100, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"srl",  TYPE_R, 0x33, 0b101, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // shift right logical
    {"sra",  TYPE_R, 0x33, 0b101, 0x20, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // shift right arithmetic
    {"or",  TYPE_R, 0x33, 0b110, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"and",  TYPE_R, 0x33, 0b111, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},

     /* ---------------- I-Type ---------------- */
    {"lb",   TYPE_I, 0x03, 0b000, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // load byte
    {"lh",   TYPE_I, 0x03, 0b001, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // load half
    {"lw",   TYPE_I, 0x03, 0b010, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // load word
    {"lbu",   TYPE_I, 0x03, 0b100, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // load byte unsigned
    {"lhu",   TYPE_I, 0x03, 0b101, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // load half unsigned

    {"addi", TYPE_I, 0x13, 0b000, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"slti", TYPE_I, 0x13, 0b010, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // set less than immediate
    {"sltiu", TYPE_I, 0x13, 0b011, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // set less than immediate unsigned
    {"xori", TYPE_I, 0x13, 0b100, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"ori", TYPE_I, 0x13, 0b110, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"andi", TYPE_I, 0x13, 0b111, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},

    {"jalr", TYPE_I, 0x67, 0b000, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // jump and link register

    /* ---------------- I7-Type ---------------- */
    {"slli", TYPE_I7, 0x13, 0b001, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // shift left logical immediate
    {"srli", TYPE_I7, 0x13, 0b101, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // shift right logical immediate
    {"srai", TYPE_I7, 0x13, 0b101, 0b0100000, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // shift right arithmetic immediate

     /* ---------------- S-Type ---------------- */
    {"sb",   TYPE_S, 0x23, 0x0, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // store byte
    {"sh",   TYPE_S, 0x23, 0x1, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // store half
    {"sw",   TYPE_S, 0x23, 0x2, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // store word

    /* ---------------- B-Type ---------------- */
    {"beq",  TYPE_B, 0x63, 0x0, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if =
    {"bne",  TYPE_B, 0x63, 0x1, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if !=
    {"blt",  TYPE_B, 0x63, 0x2, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if <
    {"bge",  TYPE_B, 0x63, 0x3, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if >=
    {"bltu",  TYPE_B, 0x63, 0x4, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if < unsigned
    {"bgeu",  TYPE_B, 0x63, 0x5, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if >= unsigned

    /* ---------------- U-Type ---------------- */
    {"lui",  TYPE_U, 0x37, 0x0, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch},
    {"auipc",  TYPE_U, 0x17, 0x0, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // add upper immediate to PC

    /* ---------------- J-Type ---------------- */
    {"jal",  TYPE_J, 0x6F, 0x0, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, //  jump and link
};

// RV64I Base Instructions (Complete set)
instr_def_t rv64i_instructions[] = {
    /* ---------------- I-Type ---------------- */
    {"ld",   TYPE_I, 0x03, 0x3, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Load double
    {"lwu",   TYPE_I, 0x03, 0x6, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Load word unsigned
    {"addiw", TYPE_I, 0x1B, 0x0, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Add immediate word

    /* ---------------- I7-Type ---------------- */
    {"slliw", TYPE_I7,0x1B, 0x1, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Shift left logical word
    {"srliw", TYPE_I7,0x1B, 0x5, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Shift right logical immediate word
    {"sraiw", TYPE_I7,0x1B, 0x5, 0x20, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Shift right arith. immediate word

    /* ---------------- S-Type ---------------- */
    {"sd",   TYPE_S, 0x23, 0x3, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // Store double

    /* ---------------- R-Type ---------------- */
    {"addw", TYPE_R, 0x3B, 0x0, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch},
    {"subw", TYPE_R, 0x3B, 0x0, 0x20, 0, ISA_RV64I, encode_dispatch, parse_dispatch},
    {"sllw", TYPE_R, 0x3B, 0x1, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // shift left logical word
    {"srlw", TYPE_R, 0x3B, 0x5, 0x00, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // shift right logical word
    {"sraw", TYPE_R, 0x3B, 0x5, 0x20, 0, ISA_RV64I, encode_dispatch, parse_dispatch}, // shift right arithmetic word


};


size_t num_rv32i_instructions = sizeof(rv32i_instructions) / sizeof(rv32i_instructions[0]);
size_t num_rv64i_instructions = sizeof(rv64i_instructions)/sizeof(rv64i_instructions[0]);
#define NUM_RV32I_INSTRUCTIONS (sizeof(rv32i_instructions) / sizeof(rv32i_instructions[0]))
#define NUM_RV64I_INSTRUCTIONS (sizeof(rv64i_instructions) / sizeof(rv64i_instructions[0]))
