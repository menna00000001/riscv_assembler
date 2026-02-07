#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "encoder.h"
#include "riscv_defs.h"

#define MAX_LINE_LEN 128

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
    instruction_t inst;
    uint32_t machine;

    while (fgets(line, sizeof(line), asm_file)) {

        /* remove newline */
        line[strcspn(line, "\n")] = 0;

        /* skip empty lines or comments */
        if (strlen(line) == 0 || line[0] == '#')
            continue;

        /* parse */
        if (!parse_line(line, &inst)) {
            printf("Parse error: %s\n", line);
            continue;
        }

        /* encode */
        switch (inst.type) {

        case INST_R:
            machine = encode_R(
                FUNCT7_OR,
                inst.rs2,
                inst.rs1,
                FUNCT3_OR,
                inst.rd,
                OPCODE_R
            );
            break;

        case INST_I:
            machine = encode_I(
                inst.imm,
                inst.rs1,
                FUNCT3_LW,
                inst.rd,
                OPCODE_LW
            );
            break;

        case INST_S:
            machine = encode_S(
                inst.imm,
                inst.rs2,
                inst.rs1,
                FUNCT3_SW,
                OPCODE_SW
            );
            break;

        case INST_B:
            machine = encode_B(
                inst.imm,
                inst.rs2,
                inst.rs1,
                FUNCT3_BEQ,
                OPCODE_BEQ
            );
            break;

        default:
            continue;
        }

        /* ---------------- output ---------------- */
        if (!byte_mode) {
            /* word-addressable */
            fprintf(hex_file, "%08X\n", machine);
        } else {
            /* byte-addressable (MSB first) */
            fprintf(hex_file, "%02X\n", (machine >> 24) & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 16) & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 8)  & 0xFF);
            fprintf(hex_file, "%02X\n",  machine        & 0xFF);
        }

        printf("%-18s -> %08X\n", line, machine);
    }

    fclose(asm_file);
    fclose(hex_file);

    printf("Assembly finished: %s -> %s (%s mode)\n",
           input_file_name, output_file_name,
           byte_mode ? "byte" : "word");

    return 0;
}
