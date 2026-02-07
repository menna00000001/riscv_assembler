#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

uint32_t encode_R(int funct7, int rs2, int rs1, int funct3, int rd, int opcode);

uint32_t encode_I(int imm, int rs1, int funct3, int rd, int opcode);

uint32_t encode_S(int imm, int rs2, int rs1, int funct3, int opcode);

uint32_t encode_B(int imm, int rs2, int rs1, int funct3, int opcode);

#endif

