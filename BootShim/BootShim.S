.global _Reset
_Reset:
 LDR sp, =stack_top
 MRC p15, 0, r0, c1, c0, 1
 BIC r0, r0, #0x40
 BIC r0, r0, #0x200000
 MCR p15, 0, r0, c1, c0, 1
 ldr r4, =0xa9000864
 ldr     r5, =0xa9000814
 orr     r6, r4, #0x200000
 str     r6, [r5, #0]
 BL CEntryPoint
 B .