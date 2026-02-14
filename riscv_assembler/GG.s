# -------------------------------------
# Setup base values
# -------------------------------------
addi x1, x0, 5           # x1 = 5
addi x2, x0, 10          # x2 = 10
addi x3, x0, -3          # x3 = -3 (sign test)
lui  x4, 0x12345         # x4 = 0x12345000
auipc x5, 0x10           # x5 = PC + 0x10000
addi x20, x5, 4          # verify auipc effect

# -------------------------------------
# Arithmetic operations
# -------------------------------------
add  x6, x1, x2          # x6 = 15
sub  x7, x2, x1          # x7 = 5
slt  x8, x3, x1          # x8 = 1 (since -3 < 5)
sltu x9, x3, x1          # x9 = 0 (unsigned compare)
addi x10, x1, -2         # x10 = 3
slti x11, x3, 0          # x11 = 1
sltiu x12, x3, 0         # x12 = 0

# -------------------------------------
# Logical operations
# -------------------------------------
and  x13, x1, x2         # x13 = 0b0101 & 0b1010 = 0
or   x14, x1, x2         # x14 = 15
xor  x15, x1, x2         # x15 = 15
andi x16, x2, 8          # x16 = 8
ori  x17, x1, 2          # x17 = 7
xori x18, x2, 1          # x18 = 11

# -------------------------------------
# Shift operations (32-bit & 64-bit)
# -------------------------------------
sll  x19, x1, x2         # shift left logical
srl  x20, x2, x1         # shift right logical
sra  x21, x3, x1         # shift right arithmetic (preserve sign)

slli x22, x1, 2          # x22 = 5 << 2 = 20
srli x23, x2, 1          # x23 = 5
srai x24, x3, 1          # x24 = -2

# 64-bit specific
addiw x25, x1, 1         # x25 = (x1 + 1) 32-bit sign extended
slliw x26, x1, 1         # x26 = (5 << 1) = 10
srliw x27, x2, 1         # x27 = (10 >> 1) = 5
sraiw x28, x3, 1         # x28 = -2 (sign-preserved)

addw  x29, x1, x2        # x29 = 15 (32-bit add)
subw  x30, x2, x1        # x30 = 5

# sign-extension test for addiw
lui   x31, 0xFFFFF
addiw x31, x31, 1        # should sign-extend properly

# -------------------------------------
# Memory test (Base address = 0)
# -------------------------------------
sw   x1, 0(x0)           # store 32-bit 5
sd   x2, 8(x0)           # store 64-bit 10
lw   x3, 0(x0)           # load 32-bit -> x3 = 5
ld   x4, 8(x0)           # load 64-bit -> x4 = 10
lwu  x5, 0(x0)           # load word unsigned

sb   x1, 16(x0)          # store byte
sh   x2, 18(x0)          # store halfword
lb   x6, 16(x0)          # load byte
lbu  x7, 16(x0)          # load byte unsigned
lh   x8, 18(x0)          # load halfword
lhu  x9, 18(x0)          # load halfword unsigned

# -------------------------------------
# Branch tests
# -------------------------------------
addi x10, x0, 1
addi x11, x0, 1
beq  x10, x11, 0          # branch if equal
addi x12, x0, 99          # might be skipped
bne  x10, x11, 0          # branch if not equal
addi x13, x0, 13
blt  x3, x2, 0            # branch if less than
addi x14, x0, 14
bge  x2, x3, 0            # branch if greater/equal
addi x15, x0, 15
bltu x0, x1, 0            # branch if less than unsigned
addi x16, x0, 16
bgeu x1, x0, 0            # branch if greater/equal unsigned
addi x17, x0, 17
addi x17, x0, 18

# -------------------------------------
# Jump tests
# -------------------------------------
jal  x18, 0                # jump
addi x19, x0, 123          # might be skipped
addi x19, x0, 321

# ----- Test JALR -----
addi x27, x0, 260          # manual address
jalr x28, 0(x27)           # x28 = return addr, jump
addi x29, x0, 333          # might be skipped
addi x29, x0, 444          # executed after JALR

# Program end
jal x0, 0                  # infinite loop / stop