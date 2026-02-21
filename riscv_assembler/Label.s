    addi x1, x0, 5        # x1 = 5
    addi x2, x0, 10       # x2 = 10

loop_start:               # <- backward branch target
    addi x3, x0, 0        # clear x3

    beq x1, x2, skip      # branch if x1 == x2 forward 12 - 24 = 12 0x00208663
    addi x3, x3, 1        # runs if branch not taken

    jal x4, loop_start    # backward jump to loop_start (negative offset) -12 0xff5ff26f

skip:
    addi x5, x0, 42       # some instruction after branch

    jalr x6, loop_start, x3  # backward jalr to loop_start using x0 as rs1 -20 0xfec18367
    addi x7, x0, 100