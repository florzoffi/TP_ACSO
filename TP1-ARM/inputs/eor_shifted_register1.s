.text
movz x1, 0xAAAA
movz x2, 0x5555
eor x0, x1, x2  // x0 = 0xFFFF
hlt