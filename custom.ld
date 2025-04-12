/* Custom linker script that selectively discards exception handling tables */
SECTIONS
{
  /* Discard exception handling tables only */
  /DISCARD/ : {
    *(.ARM.exidx*)
    *(.ARM.extab*)
  }
}

/* Include the default memory sections */
INCLUDE imxrt1062.ld