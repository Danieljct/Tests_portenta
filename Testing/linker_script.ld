.shared_ram (NOLOAD) :
{
  . = ALIGN(4);
  KEEP(*(.shared_ram))
  . = ALIGN(4);
} > SRAM_D1
