#include <stdio.h>
#include <string.h>
#include "parser.h"

/* Returns 1 if parsed successfully, 0 otherwise */
int parse_line(const char *line, instruction_t *inst)
{
    /* or x4, x5, x6 */
    if (strncmp(line, "or", 2) == 0) {
        inst->type = INST_R;
        return sscanf(line, "or x%d , x%d , x%d",
                      &inst->rd,
                      &inst->rs1,
                      &inst->rs2) == 3;
    }

    /* lw x6, -4(x9) */
    if (strncmp(line, "lw", 2) == 0) {
        inst->type = INST_I;
        return sscanf(line, "lw x%d , %d(x%d)",
                      &inst->rd,
                      &inst->imm,
                      &inst->rs1) == 3;
    }

    /* sw x6, 8(x9) */
    if (strncmp(line, "sw", 2) == 0) {
        inst->type = INST_S;
        return sscanf(line, "sw x%d , %d(x%d)",
                      &inst->rs2,
                      &inst->imm,
                      &inst->rs1) == 3;
    }

    /* beq x4, x4, -4 */
    if (strncmp(line, "beq", 3) == 0) {
        inst->type = INST_B;
        return sscanf(line, "beq x%d , x%d , %d",
                      &inst->rs1,
                      &inst->rs2,
                      &inst->imm) == 3;
    }

    return 0; /* unknown instruction */
}
