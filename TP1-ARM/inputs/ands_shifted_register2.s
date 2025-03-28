.text
movz x1, 0xAAAA
movz x2, 0x5555
ands x0, x1, x2  // x0 = 0x0000 (Z flag = 1)

HLT 0
