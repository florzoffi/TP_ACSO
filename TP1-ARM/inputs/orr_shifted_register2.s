.text
movz x1, 0x1234
movz x2, 0x00FF
orr x0, x1, x2  // x0 = 0x12FF
hlt