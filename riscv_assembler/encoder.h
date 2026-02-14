#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

uint32_t encode_R(int funct7, int rs2, int rs1, int funct3, int rd, int opcode);

uint32_t encode_I(int imm, int rs1, int funct3, int rd, int opcode);

uint32_t encode_I7(int funct7, int shamt, int rs1, int funct3, int rd, int opcode);

uint32_t encode_S(int imm, int rs2, int rs1, int funct3, int opcode);

uint32_t encode_B(int imm, int rs2, int rs1, int funct3, int opcode);

uint32_t encode_U(int imm, int rd, int opcode);

uint32_t encode_J(int imm, int rd, int opcode);


//uint32_t encode_r_type(const void *args);
//uint32_t encode_i_type(const void *args);
//uint32_t encode_or(const void *args);
//uint32_t encode_lw(const void *args);
//uint32_t encode_addi(const void *args);
//uint32_t encode_slli(const void *args);

#endif

