.text
movz x1, 0x1234
movz x2, 0x1234
cmp x1, x2        // Z flag = 1
beq igual
movz x0, 0x0000   // no se ejecuta
igual:
movz x0, 0x1111
HLT 0
