#ifndef PARSER_H
#define PARSER_H

typedef enum {
    INST_R,
    INST_I,
    INST_S,
    INST_B
} inst_type_t;

typedef struct {
    inst_type_t type;
    int rd, rs1, rs2;
    int imm;
} instruction_t;

int parse_line(const char *line, instruction_t *inst);

#endif // PARSER_H
