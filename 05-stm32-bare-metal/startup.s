/* STM32F407 Startup Code */

    .syntax unified         /* Use modern ARM syntax */
    .cpu cortex-m4          /* Target CPU */
    .thumb                  /* Thumb instruction set */

    .global Reset_Handler   /* Make visible to linker .global = Export symbol so linker can find it */

/* Vector Table - must be at 0x08000000 */
    .section .isr_vector, "a"   /* Put this in .isr_vector section, "a" = allocatable */
    .word _estack               /* Initial stack pointer (from linker script) .word = 4 byte value */
    .word Reset_Handler         /* Reset handler address */

/* Reset Handler - runs at boot */
    .section .text
    .type Reset_Handler, %function
Reset_Handler:

    /* Copy .data from flash to RAM */
    ldr r0, =_sdata         /* Destination start (RAM) ldr r0=_sdata: load address of symbol into register */
    ldr r1, =_edata         /* Destination end (RAM) */
    ldr r2, =_sidata        /* Source (flash) */

copy_data:
    cmp r0, r1              /* Are we done? */
    bge zero_bss            /* If dest >= end, move on bge=branch if greater or equal */
    ldr r3, [r2], #4        /* Load word from flash, increment source ldr... = load from address in r2, then add 4 to r2 (post-increment) */
    str r3, [r0], #4        /* Store word to RAM, increment dest str.. = store to address in r0, then add 4 to r0 */
    b copy_data             /* Loop */

    /* Zero .bss */
zero_bss:
    ldr r0, =_sbss          /* Start of .bss */
    ldr r1, =_ebss          /* End of .bss */
    mov r2, #0              /* Zero value */

zero_loop:
    cmp r0, r1              /* Are we done? */
    bge call_main           /* If start >= end, move on */
    str r2, [r0], #4        /* Store zero, increment ponter */
    b zero_loop             /* Loop */

    /* Call main */
call_main:
    bl main                 /* Branch with link to main()  (call function, save return address) */

    /* If main returns, loop forever */
hang:
    b hang