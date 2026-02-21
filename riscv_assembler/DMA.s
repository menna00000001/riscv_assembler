# ---------------- Setup addresses ----------------
lui x10, 0x80000           # x10 = SPM_BASE
addi x10, x10, 0x0

lui x11, 0xFFFF0           # x11 = DMA_CTRL_BASE (registers)
addi x11, x11, 0x0

addi x12, x0, 32           # Transfer size = 32 bytes

# ---------------- DMA: Main Memory -> SPM ----------------
addi x13, x0, 0x0          # SRC = 0x0 (main memory)
sd x13, 0(x11)             # DMA[SRC] = 0x0

sd x10, 8(x11)             # DMA[DST] = SPM_BASE
sd x12, 16(x11)            # DMA[SIZE] = 32 bytes

addi x14, x0, 1             # CTRL[0] = START
sd x14, 24(x11)             # DMA[CTRL] = START

# ---------------- CPU reads SPM BEFORE DONE (hazard) ----------------
ld x20, 0(x10)             # CPU tries to read SPM[0] early
lw x21, 16(x10)            # CPU tries to read SPM[16] early