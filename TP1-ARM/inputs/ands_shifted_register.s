.text
movz x1, 0xF0F0
movz x2, 0x0F0F    
ands x0, x1, x2  // x0 = 0x0000
HLT 0