#ifndef RISCV_DEFS
#define RISCV_DEFS


/* Opcodes */
#define OPCODE_LW   0x03
#define OPCODE_SW   0x23
#define OPCODE_R    0x33
#define OPCODE_BEQ  0x63

/* funct3 */
#define FUNCT3_LW   0x2
#define FUNCT3_SW   0x2
#define FUNCT3_OR   0x6
#define FUNCT3_BEQ  0x0

/* funct7 */
#define FUNCT7_OR   0x00


#endif // RISCV_DEFS
