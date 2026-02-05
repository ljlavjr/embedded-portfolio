/* STM32F407VG Memory Layout */
/*
    MEMORY {}:  Defines physical memory regions on the chip
    FLASH (rx): Flash memory, readable(r) and executable(x). Code lives here
    RAM (rwx):  RAM, readable, writable, executable. Variables live here
    ORIGIN:     Starting address. These addresses come from the STM32F407 datasheet/reference manual
    LENGTH:     Size of the region
*/
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Entry point - first function to run after reset */
ENTRY(Reset_Handler)

/* Section definitions - where to place code and data */
SECTIONS
{
    /*  Vector table must be first in Flash */
    /*    Cortex-M looks at address 0x08000000 on boot
        First 4 bytes is the Initial stack pointer value
        Next 4 bytes is the Address of Reset_Handler

        .isr_vector:    Section name for the interrupt/reset vector table
        KEEP():         Do not discard this even if it looks unused. Critical for boot
        *(isr_vector):  Take the .isr_vector section from all input files (*)
        > FLASH:        Place this in the FLASH memory region
    */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    /* Program code */
    /*  
        .text:      Standard section name for executable code
        *(.text):   All .text sections from all object files
        *(.text*):  Also grab .text.startup, .text.main, etc. (compiler sometimes creates subsections)
        > FLASH:    Code goes in flash (it is read only, runs in place)
    */
    .text :
    {
        *(.text)
        *(.text*)
    } > FLASH

    /* Read-only data (const variables, string literals) */
    /* Goes in FLASH because it never changes, saves RAM */
    .rodata :
    {
        *(.rodata)
        *(.rodata*)
    } > FLASH

    /* Initialized data - stored in flash, copied to RAM at startup */
    /*  
        _sidata = LOADADDR(.data):  Symbol marking where .data is stored in flash
        _sdata=. :                  Symbol marking start of .data in RAM (the . means "current address")
        _edata=. :                  Symbol marking end of .data in RAM
        > RAM AT > FLASH:           Lives in RAM, but loaded from flash
    */
    _sidata = LOADADDR(.data);

    .data :
    {
        _sdata = .;
        *(.data)
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    /* Uninitialized data - zeroed at startup */
    /*
        .bss:           Uninitialized global/static variables. C standard says they default to zero
        _(COMMON):      Old-style uninitialized globals (some compilers use this)
        _sbss/_ebss:    Start/end symbols so startup code knows what to zero
        > RAM:          Nothing stored in flash. Startup code just fills this region with zeros
    */
    .bss : 
    {
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    /* End of RAM - used for initial stack pointer */
    /*
        _estack:    Top of RAM. Stack grows downward, so we start at the top. This value goes in the first 4 bytes of the vector table
    */
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}