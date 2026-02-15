#include "encoder.h"

/* R-type: or */
uint32_t encode_R(int funct7, int rs2, int rs1, int funct3, int rd, int opcode)
{
    return ((funct7 & 0x7F) << 25) |
           ((rs2   & 0x1F) << 20) |
           ((rs1   & 0x1F) << 15) |
           ((funct3& 0x07) << 12) |
           ((rd    & 0x1F) << 7 ) |
           (opcode & 0x7F);
}

/* I-type: lw */
uint32_t encode_I(int imm, int rs1, int funct3, int rd, int opcode)
{
    return ((imm   & 0xFFF) << 20) |
           ((rs1   & 0x1F ) << 15) |
           ((funct3& 0x07 ) << 12) |
           ((rd    & 0x1F ) << 7 ) |
           (opcode & 0x7F);
}

/* I7-type encoder: e.g., slli, srli, srai */
//| funct7 (7b) | shamt (5b) | rs1 (5b) | funct3 (3b) | rd (5b) | opcode (7b) |
uint32_t encode_I7(int funct7, int shamt, int rs1, int funct3, int rd, int opcode)
{
    return ((funct7 & 0x7F) << 25) |   // bits 31:25
           ((shamt  & 0x1F) << 20) |   // bits 24:20
           ((rs1    & 0x1F) << 15) |   // bits 19:15
           ((funct3 & 0x07) << 12) |   // bits 14:12
           ((rd     & 0x1F) << 7 ) |   // bits 11:7
           (opcode  & 0x7F);           // bits 6:0
}

/* S-type: sw */
uint32_t encode_S(int imm, int rs2, int rs1, int funct3, int opcode)
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
uint32_t encode_B(int imm, int rs2, int rs1, int funct3, int opcode)
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

/* U-type: lui / auipc */
// imm[31:12] | rd[11:7] | opcode[6:0]
uint32_t encode_U(int imm, int rd, int opcode)
{
    return ((imm & 0xFFFFF) << 12) |  // upper 20 bits go into [31:12]
           ((rd  & 0x1F) << 7)   |
           (opcode & 0x7F);
}

/* J-type: jal */
//imm[20]    -> bit 31
//imm[10:1]  -> bits 30:21
//imm[11]    -> bit 20
//imm[19:12] -> bits 19:12
//rd[11:7]   -> bits 11:7
//opcode[6:0] -> bits 6:0
uint32_t encode_J(int imm, int rd, int opcode)
{
    uint32_t imm20    = (imm >> 20) & 0x1;
    uint32_t imm10_1  = (imm >> 1)  & 0x3FF;
    uint32_t imm11    = (imm >> 11) & 0x1;
    uint32_t imm19_12 = (imm >> 12) & 0xFF;

    return (imm20 << 31)      |
           (imm19_12 << 12)   |
           (imm11 << 20)      |
           (imm10_1 << 21)    |
           ((rd & 0x1F) << 7)|
           (opcode & 0x7F);
}

