.global _ZN3TCB13contextSwitchEPNS_7ContextES1_
.type _ZN3TCB13contextSwitchEPNS_7ContextES1_, @function
_ZN3TCB13contextSwitchEPNS_7ContextES1_:
    #da li je bitno sto mi swapuju mesto sp1 i sp2?
    sd ra, 0 * 8(a0)
   #store ra na ra mesto
    sd sp, 2 * 8(a0)
    #store ssp na ssp mesto
    csrr ra, sscratch
    #store mv ra, sp
    sd ra, 1 * 8(a0)
    #store sp na sp mesto

    ld ra, 1 * 8(a1)
    #load ra, sp
    csrw sscratch, ra
    #load sscratch sp
    ld ra, 0 * 8(a1)
    #load ra ra
    ld sp, 2 * 8(a1)
    #load ssp, ssp

    ret