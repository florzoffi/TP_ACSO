.text
movz x1, 0xFFFF
movz x2, 0x00FF
ands x0, x1, x2  // x0 = 0x00FF
hlt