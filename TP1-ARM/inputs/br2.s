.text
movz x0, 0xDEAD
movz x1, 0x0010         // apunta a 0x00400010
br x1
movz x0, 0xBEEF         // no se ejecuta
movz x2, 0x1111         // en 0x00400010
HLT 0
