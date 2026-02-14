#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "encoder.h"
#include "instruction_defs.h"
#include "instruction_args.h"

#define MAX_LINE_LEN 128

/* Function prototypes */
instr_def_t *find_instruction(const char *mnemonic);
int parse_operands(const char *operands, instr_def_t *def, instr_args_t *args);

int main(int argc, char *argv[])
{
    /* ---------------- argument check ---------------- */
    if (argc != 4) {
        printf("Usage: %s <input_file.s> <output_file.hex> <word|byte>\n", argv[0]);
        return 1;
    }

    const char *input_file_name  = argv[1];
    const char *output_file_name = argv[2];
    const char *mode             = argv[3];

    int byte_mode = 0;

    if (strcmp(mode, "byte") == 0) {
        byte_mode = 1;
    } else if (strcmp(mode, "word") == 0) {
        byte_mode = 0;
    } else {
        printf("Invalid mode: %s (use 'word' or 'byte')\n", mode);
        return 1;
    }

    /* ---------------- file open ---------------- */
    FILE *asm_file = fopen(input_file_name, "r");
    if (!asm_file) {
        perror("Cannot open input file");
        return 1;
    }

    FILE *hex_file = fopen(output_file_name, "w");
    if (!hex_file) {
        perror("Cannot open output file");
        fclose(asm_file);
        return 1;
    }

    /* ---------------- assemble loop ---------------- */
    char line[MAX_LINE_LEN];
    char line_copy[MAX_LINE_LEN];
    uint32_t machine;

while (fgets(line, sizeof(line), asm_file)) {
    /* remove newline */
    line[strcspn(line, "\n")] = 0;
    strcpy(line_copy, line);  // Keep original for error messages

    /* Remove inline comments */
    char *comment = strchr(line, '#');
    if (comment) *comment = '\0';

    /* Trim leading whitespace */
    char *ptr = line;
    while (isspace(*ptr)) ptr++;

    /* Trim trailing whitespace */
    char *end = ptr + strlen(ptr) - 1;
    while (end > ptr && isspace(*end)) *end-- = '\0';

    /* skip empty lines */
    if (strlen(ptr) == 0)
        continue;

    /* ---------------- Parse mnemonic and operands ---------------- */
    char mnemonic[32] = {0};
    char operands[128] = {0};

    /* Extract mnemonic (first word) */
    int i = 0;
    while (*ptr && !isspace(*ptr) && i < 31) {
        mnemonic[i++] = *ptr++;
    }
    mnemonic[i] = '\0';

    /* Skip whitespace to operands */
    while (isspace(*ptr)) ptr++;

    /* Copy remaining as operands */
    strncpy(operands, ptr, sizeof(operands) - 1);
    operands[sizeof(operands) - 1] = '\0';

    /* ---------------- Find instruction ---------------- */
    instr_def_t *def = find_instruction(mnemonic);
    if (!def) {
        printf("Unknown instruction: %s\n", line_copy);
        continue;
    }

    /* ---------------- Parse operands ---------------- */
    instr_args_t args = {0};
    if (!parse_operands(operands, def, &args)) {
        printf("Parse error: %s\n", line_copy);
        continue;
    }

    /* ---------------- Encode instruction ---------------- */
    machine = def->encoder(def, &args);

        /* ---------------- output ---------------- */
        if (!byte_mode) {
            /* word-addressable */
            fprintf(hex_file, "%08X\n", machine);
        } else {
            /* byte-addressable (LSB first) */
            fprintf(hex_file, "%02X\n",  machine        & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 8)  & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 16) & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 24) & 0xFF);
        }

        printf("%-18s -> %08X\n", line_copy, machine);
    }

    fclose(asm_file);
    fclose(hex_file);

    printf("Assembly finished: %s -> %s (%s mode)\n",
           input_file_name, output_file_name,
           byte_mode ? "byte" : "word");

    return 0;
}

/* Find instruction in all instruction sets */
instr_def_t *find_instruction(const char *mnemonic) {
    // RV32I
    extern instr_def_t rv32i_instructions[];
    extern size_t num_rv32i_instructions;

    for (size_t i = 0; i < num_rv32i_instructions; i++)
        if (strcmp(rv32i_instructions[i].mnemonic, mnemonic) == 0)
            return &rv32i_instructions[i];

    // RV64I
    extern instr_def_t rv64i_instructions[];
    extern size_t num_rv64i_instructions;

    for (size_t i = 0; i < num_rv64i_instructions; i++)
        if (strcmp(rv64i_instructions[i].mnemonic, mnemonic) == 0)
            return &rv64i_instructions[i];

    // RV32M
    //extern instr_def_t rv32m_instructions[];
    //extern size_t num_rv32m_instructions;

    //for (size_t i = 0; i < num_rv32m_instructions; i++)
     //   if (strcmp(rv32m_instructions[i].mnemonic, mnemonic) == 0)
       //     return &rv32m_instructions[i];

    return NULL; // Not found in any set
}

/* Parse operands based on instruction type and format */
int parse_operands(const char *operands, instr_def_t *def, instr_args_t *args) {
    char format[128];
    strncpy(format, operands, sizeof(format)-1);
    format[sizeof(format)-1] = '\0';

    switch (def->format) {

        case TYPE_R:
            /* Format: rd, rs1, rs2 */
            if (sscanf(format, "x%d, x%d, x%d",
                       &args->rd, &args->rs1, &args->rs2) == 3) {
                return 1;
            }
            break;

        case TYPE_I:
            if (def->opcode == 0x03) { /* Load instructions */
                /* Format: rd, offset(rs1) */
                if (sscanf(format, "x%d, %i(x%d)",
                           &args->rd, &args->imm, &args->rs1) == 3) return 1;
            } else if (def->opcode == 0x13) { /* ALU immediate */
                /* Format: rd, rs1, imm */
                if (sscanf(format, "x%d, x%d, %i",
                           &args->rd, &args->rs1, &args->imm) == 3) return 1;
            } else if (def->opcode == 0x67) { /* JALR */
                /* Format: rd, rs1, offset */
                if (sscanf(format, "x%d, x%d, %i",
                           &args->rd, &args->rs1, &args->imm) == 3) return 1;
            }
            break;

        case TYPE_I7:  /* Shift Immediate with funct7 */
            /* Format: rd, rs1, shamt */
            if (sscanf(format, "x%d, x%d, %i",
                       &args->rd, &args->rs1, &args->shamt) == 3) return 1;
            break;

        case TYPE_S:
            /* Format: rs2, offset(rs1) */
            if (sscanf(format, "x%d, %i(x%d)",
                       &args->rs2, &args->imm, &args->rs1) == 3) return 1;
            break;

        case TYPE_B:
            /* Format: rs1, rs2, offset */
            if (sscanf(format, "x%d, x%d, %i",
                       &args->rs1, &args->rs2, &args->imm) == 3) return 1;
            break;

        case TYPE_U:
            /* Format: rd, imm */
            if (sscanf(format, "x%d, %i", &args->rd, &args->imm) == 2) return 1;
            break;

        case TYPE_J:
            /* Format: rd, offset */
            if (sscanf(format, "x%d, %i", &args->rd, &args->imm) == 2) return 1;
            break;

        default:
            break;
    }

    return 0; /* Parsing failed */
}
