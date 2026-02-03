#include "encoder.h"

/* R-type: or */
uint32_t encode_R(int funct7, int rs2, int rs1,
                  int funct3, int rd, int opcode)
{
    return ((funct7 & 0x7F) << 25) |
           ((rs2   & 0x1F) << 20) |
           ((rs1   & 0x1F) << 15) |
           ((funct3& 0x07) << 12) |
           ((rd    & 0x1F) << 7 ) |
           (opcode & 0x7F);
}

/* I-type: lw */
uint32_t encode_I(int imm, int rs1,
                  int funct3, int rd, int opcode)
{
    return ((imm   & 0xFFF) << 20) |
           ((rs1   & 0x1F ) << 15) |
           ((funct3& 0x07 ) << 12) |
           ((rd    & 0x1F ) << 7 ) |
           (opcode & 0x7F);
}

/* S-type: sw */
uint32_t encode_S(int imm, int rs2, int rs1,
                  int funct3, int opcode)
{
    uint32_t imm11_5 = (imm >> 5) & 0x7F;
    uint32_t imm4_0  = imm & 0x1F;

    return (imm11_5 << 25) |
           ((rs2   & 0x1F) << 20) |
           ((rs1   & 0x1F) << 15) |
           ((funct3& 0x07) << 12) |
           (imm4_0 << 7) |
           (opcode & 0x7F);
}

/* B-type: beq */
uint32_t encode_B(int imm, int rs2, int rs1,
                  int funct3, int opcode)
{
    uint32_t imm12   = (imm >> 12) & 0x1;
    uint32_t imm10_5 = (imm >> 5)  & 0x3F;
    uint32_t imm4_1  = (imm >> 1)  & 0xF;
    uint32_t imm11   = (imm >> 11) & 0x1;

    return (imm12 << 31) |
           (imm10_5 << 25) |
           ((rs2   & 0x1F) << 20) |
           ((rs1   & 0x1F) << 15) |
           ((funct3& 0x07) << 12) |
           (imm4_1 << 8) |
           (imm11 << 7) |
           (opcode & 0x7F);
}
