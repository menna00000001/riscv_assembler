#include "instruction_args.h"
#include "instruction_defs.h"
#include "encoder.h"
#include "riscv_instructions.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern int find_label(const char *name, uint32_t *address);

int is_number(const char *str) {
    if (*str == '-' || *str == '+')
        str++;

    if (*str == '\0')
        return 0;

    while (*str) {
        if (!isdigit(*str) && *str != 'x' && !isxdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

int parse_number(const char *str) {
    return (int)strtol(str, NULL, 0);  // supports decimal & 0x hex
}

typedef struct {
    const char *name;
    uint16_t addr;
} csr_def_t;

csr_def_t csr_table[] = {
    {"mtvec",  0x305},
    {"mepc",   0x341},
    {"mcause", 0x342},
    {"mtval",  0x343},
};

#define NUM_CSR (sizeof(csr_table)/sizeof(csr_table[0]))

int lookup_csr(const char *name, uint16_t *out) {
    for (int i = 0; i < NUM_CSR; i++) {
        if (strcmp(name, csr_table[i].name) == 0) {
            *out = csr_table[i].addr;
            return 1;
        }
    }
    return 0;
}

void error(const char *msg) {
    printf("Error: %s\n", msg);
}

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
            // Normal R-Type
            return sscanf(line,"x%d, x%d, x%d", &a->rd, &a->rs1, &a->rs2);

        case TYPE_I:{
            if (def->opcode == 0x03) { /* Load instructions */
                /* Format: rd, offset(rs1) */
                return sscanf(line,"x%d, %i(x%d)",&a->rd, &a->imm, &a->rs1);
            } else if (def->opcode == 0x13 || def->opcode == 0x1B || def->opcode == 0x67) { /* ALU immediate & JALR */
                /* Format: rd, rs1, imm */
                return sscanf(line,"x%d, x%d, %i", &a->rd, &a->rs1, &a->imm);
            }
            // ZICSR //
            else if (def->opcode == 0x73 && def->funct3 == 0) {   // SYSTEM
                // ecall / ebreak / mret
                a->rd  = 0;           // always zero
                a->rs1 = 0;           // always zero
                a->imm = def->funct12; // imm = funct12
                return 1;
                }
            else if (def->funct3 == 1 || def->funct3 == 2 || def->funct3 == 3) {
                    char csr[32];
                    // CSR register form
                    // csrrw rd,offset,rs1
                    int ret = sscanf(line,"x%d, %31[^,], x%d", &a->rd, csr, &a->rs1);
                        if (ret == 3) {
                            if (is_number(csr)) {
                                a->imm = parse_number(csr);
                        } else if (!lookup_csr(csr, (uint16_t*)&a->imm)) {
                                error("Unknown CSR");
                            }
                        }
                    return ret;
                }
            else if (def->funct3 == 5 || def->funct3 == 6 || def->funct3 == 7) {
                    char csr[32];
                    // CSR immediate form
                    // csrrwi rd,offset,uimm                          //uimm
                    int ret = sscanf(line,"x%d, %31[^,], %d", &a->rd, csr, &a->rs1);
                    if (ret == 3) {
                        if (is_number(csr)) {
                            a->imm = parse_number(csr);
                    } else if (!lookup_csr(csr, (uint16_t*)&a->imm)) {
                            error("Unknown CSR");
                    }
                }
        return ret;
                }
            }

        case TYPE_I7:
            return sscanf(line,"x%d, x%d, %i", &a->rd, &a->rs1, &a->shamt);

        case TYPE_S:
            return sscanf(line,"x%d, %i(x%d)", &a->rs2, &a->imm, &a->rs1);

        case TYPE_B: {
            char label[64];

            if (sscanf(line,"x%d, x%d, %i",&a->rs1, &a->rs2, &a->imm) == 3)
                return 1;

            if (sscanf(line,"x%d, x%d, %63s",&a->rs1, &a->rs2, label) == 3) {

            uint32_t target;

            if (!find_label(label, &target)) {
                printf("Unknown label: %s\n", label);
                return 0;
            }

            a->imm = (int32_t)target - (int32_t)a->current_pc;

            if (a->imm % 2 != 0) {
                printf("Unaligned branch target\n");
                return 0;
            }

            if (a->imm < -4096 || a->imm > 4094) {
                printf("Branch offset out of range\n");
                return 0;
            }

            return 1;
        }

        return 0;
        }

        case TYPE_U:
            return sscanf(line, "x%d, %i", &a->rd, &a->imm);

        case TYPE_J: {
            char label_name[64];

            // Immediate number
            if (sscanf(line, "x%d, %i", &a->rd, &a->imm) == 2)
                return 1;

            // Label target
            if (sscanf(line, "x%d, %63s", &a->rd, label_name) == 2) {
                uint32_t target_addr;
                if (!find_label(label_name, &target_addr)) {
                    printf("Unknown label: %s\n", label_name);
                        return 0;
                    }

        a->imm = (int32_t)target_addr - (int32_t)a->current_pc;

        // Check alignment
        if (a->imm % 2 != 0) {
            printf("Unaligned jump target\n");
            return 0;
        }

        if (a->imm < -1048576 || a->imm > 1048574) {
            printf("Jump offset out of range\n");
            return 0;
        }

        return 1;
    }

    return 0;
}

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
    {"blt",  TYPE_B, 0x63, 0x4, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if <
    {"bge",  TYPE_B, 0x63, 0x5, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if >=
    {"bltu",  TYPE_B, 0x63, 0x6, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if < unsigned
    {"bgeu",  TYPE_B, 0x63, 0x7, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // branch if >= unsigned

    /* ---------------- U-Type ---------------- */
    {"lui",  TYPE_U, 0x37, 0x0, 0x00, 0, ISA_RV32I, encode_dispatch, parse_dispatch}, // load upper immediate
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
// M Extension
instr_def_t m_instructions[] = {

    /* ----------- R-Type (RV32 + RV64) ----------- */
    {"mul",    TYPE_R, 0x33, 0b000, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"mulh",   TYPE_R, 0x33, 0b001, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"mulhsu", TYPE_R, 0x33, 0b010, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"mulhu",  TYPE_R, 0x33, 0b011, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"div",    TYPE_R, 0x33, 0b100, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"divu",   TYPE_R, 0x33, 0b101, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"rem",    TYPE_R, 0x33, 0b110, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"remu",   TYPE_R, 0x33, 0b111, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},

    /* ----------- RV64 Only (Word ops) ----------- */
    {"mulw",   TYPE_R, 0x3B, 0b000, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"divw",   TYPE_R, 0x3B, 0b100, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"divuw",  TYPE_R, 0x3B, 0b101, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"remw",   TYPE_R, 0x3B, 0b110, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
    {"remuw",  TYPE_R, 0x3B, 0b111, 0x01, 0, ISA_EXT_M, encode_dispatch, parse_dispatch},
};

// Zicsr Extension (not complete)
instr_def_t zicsr_instructions[] = {
    // ecall / ebreak / mret
    {"ecall",  TYPE_I, 0x73, 0x0, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    {"ebreak", TYPE_I, 0x73, 0x0, 0x00, 0b000000000001, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    {"mret",   TYPE_I, 0x73, 0x0, 0x18, 0b001100000010, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    // CSR register form
    {"csrrw",   TYPE_I, 0x73, 0x1, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    {"csrrs",   TYPE_I, 0x73, 0x2, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    {"csrrc",   TYPE_I, 0x73, 0x3, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    // CSR immediate form
    {"csrrwi",   TYPE_I, 0x73, 0x5, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    {"csrrsi",   TYPE_I, 0x73, 0x6, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
    {"csrrci",   TYPE_I, 0x73, 0x7, 0x00, 0, ISA_EXT_ZICSR, encode_dispatch, parse_dispatch},
};

size_t num_rv32i_instructions = sizeof(rv32i_instructions) / sizeof(rv32i_instructions[0]);
size_t num_rv64i_instructions = sizeof(rv64i_instructions)/sizeof(rv64i_instructions[0]);
size_t num_m_instructions = sizeof(m_instructions)/sizeof(m_instructions[0]);
size_t num_zicsr_instructions = sizeof(zicsr_instructions)/sizeof(zicsr_instructions[0]);
#define NUM_RV32I_INSTRUCTIONS (sizeof(rv32i_instructions) / sizeof(rv32i_instructions[0]))
#define NUM_RV64I_INSTRUCTIONS (sizeof(rv64i_instructions) / sizeof(rv64i_instructions[0]))
#define NUM_M_INSTRUCTIONS (sizeof(m_instructions)/sizeof(m_instructions[0]))
#define NUM_ZICSR_INSTRUCTIONS (sizeof(zicsr_instructions)/sizeof(zicsr_instructions[0]))
