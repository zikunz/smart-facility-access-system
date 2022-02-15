--retain=g_pfnVectors

//*****************************************************************************
// The following command line options are set as part of the CCS project.  
// If you are building using the command line, or for some reason want to    
// define them here, you can uncomment and modify these lines as needed.     
// If you are using CCS for building, it is probably better to make any such 
// modifications in your CCS project and leave this file alone.              
//*****************************************************************************


//*****************************************************************************
// The starting address of the application.  Normally the interrupt vectors  
// must be located at the beginning of the application.                      
//*****************************************************************************
#define RAM_BASE 0x20004000

/* System memory map */

MEMORY
{
    /* Application uses internal RAM for program and data */
    /* RAM Blocks are modified for CC3200 ES 1.33 (XCC3200JR) which supports 240KB (256-16) APP RAM size */
    SRAM_CODE (RWX) : origin = 0x20004000, length = 0x19000  /* 100 KB */
    SRAM_DATA (RWX) : origin = 0x2001D000, length = 0x23000  /* 140 KB */
}

/* Section allocation in memory */

SECTIONS
{
    .intvecs:   > RAM_BASE
    .init_array : > SRAM_CODE
    .vtable :   > SRAM_CODE
    .text   :   > SRAM_CODE
    .const  :   > SRAM_CODE
    .cinit  :   > SRAM_CODE
    .pinit  :   > SRAM_CODE
    .data   :   > SRAM_DATA
    .bss    :   > SRAM_DATA
    .sysmem :   > SRAM_DATA
    .stack  :   > SRAM_DATA(HIGH)
}

