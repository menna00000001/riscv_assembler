#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "encoder.h"
#include "instruction_defs.h"
#include "instruction_args.h"

#define MAX_LINE_LEN 128
#define MAX_LABELS 256
#define MAX_LABEL_LEN 64

/* ---------------------- Types ---------------------- */
typedef struct {
    char name[MAX_LABEL_LEN];
    uint32_t address;
} label_t;

/* ---------------------- Globals ---------------------- */
label_t label_table[MAX_LABELS];
int label_count = 0;

/* ---------------------- Function prototypes ---------------------- */
instr_def_t *find_instruction(const char *mnemonic);
int parse_operands(const char *operands, instr_def_t *def, instr_args_t *args);
int find_label(const char *name, uint32_t *address);

/* ---------------------- Main ---------------------- */
int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <input_file.s> <output_file.hex> <word|byte>\n", argv[0]);
        return 1;
    }

    const char *input_file_name  = argv[1];
    const char *output_file_name = argv[2];
    const char *mode             = argv[3];

    int byte_mode = (strcmp(mode, "byte") == 0);

    FILE *asm_file = fopen(input_file_name, "r");
    if (!asm_file) { perror("Cannot open input file"); return 1; }

    FILE *hex_file = fopen(output_file_name, "w");
    if (!hex_file) { perror("Cannot open output file"); fclose(asm_file); return 1; }

    char line[MAX_LINE_LEN];
    char line_copy[MAX_LINE_LEN];
    uint32_t pc = 0;

    /* ---------------------- First pass: collect labels ---------------------- */
    while (fgets(line, sizeof(line), asm_file)) {
        line[strcspn(line, "\n")] = 0; // remove newline

        char *ptr = line;
        while (isspace(*ptr)) ptr++;  // trim leading space
        if (*ptr == '\0' || *ptr == '#') continue; // skip empty/comment lines

        char *colon = strchr(ptr, ':');
        if (colon) {
            *colon = '\0';
            if (label_count < MAX_LABELS) {
                strcpy(label_table[label_count].name, ptr);
                label_table[label_count].address = pc;
                label_count++;
            } else {
                printf("Label table full!\n");
                return 1;
            }
            continue; // label-only line
        }

        pc += 4; // increment PC per instruction
    }

    rewind(asm_file);
    pc = 0; // reset PC for second pass

    /* ---------------------- Second pass: encode instructions ---------------------- */
    while (fgets(line, sizeof(line), asm_file)) {
        line[strcspn(line, "\n")] = 0;
        strcpy(line_copy, line);

        char *ptr = line;
        while (isspace(*ptr)) ptr++;  // trim leading space
        if (*ptr == '\0' || *ptr == '#') continue; // skip empty/comment lines

        // skip label-only lines
        char *colon = strchr(ptr, ':');
        if (colon) continue;

        // trim trailing whitespace
        char *end = ptr + strlen(ptr) - 1;
        while (end > ptr && isspace(*end)) *end-- = '\0';
        if (strlen(ptr) == 0) continue;

        /* Extract mnemonic + operands */
        char mnemonic[32] = {0};
        char operands[128] = {0};

        int i = 0;
        while (*ptr && !isspace(*ptr) && i < 31) mnemonic[i++] = *ptr++;
        mnemonic[i] = '\0';

        while (isspace(*ptr)) ptr++;
        strncpy(operands, ptr, sizeof(operands) - 1);
        operands[sizeof(operands)-1] = '\0';

        /* Find instruction */
        instr_def_t *def = find_instruction(mnemonic);
        if (!def) {
            printf("Unknown instruction: %s\n", line_copy);
            continue;
        }

        /* Parse operands */
        instr_args_t args = {0};
        args.current_pc = pc; // assign PC before parsing

        if (!def->parser(def, operands, &args)) {
            printf("Parse error: %s\n", line_copy);
            continue;
        }

        /* Encode instruction */
        uint32_t machine = def->encoder(def, &args);

        /* Output to file */
        if (!byte_mode) {
            fprintf(hex_file, "%08X\n", machine);
        } else {
            fprintf(hex_file, "%02X\n",  machine        & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 8)  & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 16) & 0xFF);
            fprintf(hex_file, "%02X\n", (machine >> 24) & 0xFF);
        }

        printf("%-18s -> %08X\n", line_copy, machine);

        pc += 4; // increment PC
    }

    fclose(asm_file);
    fclose(hex_file);

    printf("Assembly finished: %s -> %s (%s mode)\n",
           input_file_name, output_file_name,
           byte_mode ? "byte" : "word");

    return 0;
}

/* ---------------------- Find instruction ---------------------- */
instr_def_t *find_instruction(const char *mnemonic) {
    extern instr_def_t rv32i_instructions[];
    extern size_t num_rv32i_instructions;

    for (size_t i = 0; i < num_rv32i_instructions; i++)
        if (strcmp(rv32i_instructions[i].mnemonic, mnemonic) == 0)
            return &rv32i_instructions[i];

    extern instr_def_t rv64i_instructions[];
    extern size_t num_rv64i_instructions;

    for (size_t i = 0; i < num_rv64i_instructions; i++)
        if (strcmp(rv64i_instructions[i].mnemonic, mnemonic) == 0)
            return &rv64i_instructions[i];

    extern instr_def_t zicsr_instructions[];
    extern size_t num_zicsr_instructions;

    for (size_t i = 0; i < num_zicsr_instructions; i++)
        if (strcmp(zicsr_instructions[i].mnemonic, mnemonic) == 0)
            return &zicsr_instructions[i];

    return NULL; // Not found
}

/* ---------------------- Find label ---------------------- */
int find_label(const char *name, uint32_t *address) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(label_table[i].name, name) == 0) {
            *address = label_table[i].address;
            return 1;
        }
    }
    return 0;
}
