.text
movz x1, 0x1111
b skip
movz x1, 0x2222  // no se ejecuta
skip:
HLT 0
