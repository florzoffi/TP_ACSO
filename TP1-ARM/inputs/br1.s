.text
movz x0, 0xAAAA         // en 0x00400000
movz x1, 0x000C         // en 0x00400004
br x1                  // salta a 0x0040000C
movz x0, 0xBBBB         // en 0x00400008 (no se ejecuta)
movz x2, 0xCCCC         // en 0x0040000C
hlt