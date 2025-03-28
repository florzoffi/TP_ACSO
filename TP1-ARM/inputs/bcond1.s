.text
movz x1, 0x1234
movz x2, 0x0000
cmp x1, x2        // Z flag = 0
bne distinto
movz x0, 0x0000   // no se ejecuta
distinto:
movz x0, 0x2222
hlt