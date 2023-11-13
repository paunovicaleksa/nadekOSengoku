.extern _ZN5Riscv20handleSupervisorTrapEv

.align 4
.global _ZN5Riscv14supervisorTrapEv
.type _ZN5Riscv14supervisorTrapEv, @function
_ZN5Riscv14supervisorTrapEv:
    # push all registers to stack
    # treba raditi neke gluposti gde treba zameniti scratch i sp?
    # mislim da ce ovako moci, otprilike:
    addi sp, sp, -16
    sd s0, 1*8(sp)
    sd ra, 0*8(sp)
    mv s0, sp
    csrr ra, sscratch
    addi sp, sp, 16
    csrw sscratch, sp
    mv sp, ra
    ld ra, 0*8(s0)
    ld s0, 1*8(s0)


    addi sp, sp, -256
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
     sd x\index, \index * 8(sp)
    .endr


    call _ZN5Riscv20handleSupervisorTrapEv

    # pop all registers from stack

    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
     ld x\index, \index * 8(sp)
    .endr
    addi sp, sp, 256

    addi sp, sp, -16 #ovo je sistemski stek sad, u sscratch je context
    sd s0, 1*8(sp)
    sd ra, 0*8(sp)
    mv s0, sp
    addi sp, sp, 16
    csrr ra, sscratch
    csrw sscratch, sp
    mv sp, ra
    ld ra, 0*8(s0)
    ld s0, 1*8(s0)


    sret