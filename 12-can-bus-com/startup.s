/* STM32F407 Startup Code */

    .syntax unified         /* Use modern ARM syntax */
    .cpu cortex-m4          /* Target CPU */
    .thumb                  /* Thumb instruction set */

    .global Reset_Handler   /* Make visible to linker .global = Export symbol so linker can find it */

/* Vector Table - must be at 0x08000000 */
    .section .isr_vector, "a"
    .word _estack               /* 0: Initial stack pointer */
    .word Reset_Handler         /* 1: Reset */
    .word 0                     /* 2: NMI */
    .word HardFault_Handler                     /* 3: HardFault */
    .word 0                     /* 4: MemManage */
    .word 0                     /* 5: BusFault */
    .word 0                     /* 6: UsageFault */
    .word 0                     /* 7: Reserved */
    .word 0                     /* 8: Reserved */
    .word 0                     /* 9: Reserved */
    .word 0                     /* 10: Reserved */
    .word SVC_Handler           /* 11: SVCall */
    .word 0                     /* 12: Debug Monitor */
    .word 0                     /* 13: Reserved */
    .word PendSV_Handler        /* 14: PendSV */
    .word SysTick_Handler       /* 15: SysTick */

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

    /* Enable FPU - required for -mfloat-abi=hard */
    /* Set CP10 and CP11 to full access in CPACR */
enable_fpu:
    ldr r0, =0xE000ED88       /* SCB->CPACR address */
    ldr r1, [r0]
    orr r1, r1, #(0xF << 20)  /* Enable CP10 and CP11 (bits 20-23) */
    str r1, [r0]
    dsb                        /* Data sync barrier */
    isb                        /* Instruction sync barrier */

    /* Call main */
call_main:
    bl main                 /* Branch with link to main()  (call function, save return address) */

    /* If main returns, loop forever */
hang:
    b hang
    
    .type HardFault_Handler, %function
    .global HardFault_Handler
HardFault_Handler:
    ldr r0, =0x40020C18
    ldr r1, =0x00008000
    str r1, [r0]
    b HardFault_Handler
    