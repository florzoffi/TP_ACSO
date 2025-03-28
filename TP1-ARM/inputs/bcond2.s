.text
movz x1, 0x0005
movz x2, 0x0002
cmp x1, x2        // x1 > x2 → Z = 0, N = 0
bgt mayor
movz x0, 0x0000
mayor:
movz x0, 0x3333
hlt