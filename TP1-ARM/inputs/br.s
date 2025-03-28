.text
movz x1, 0x0008         // br a 0x00400008
br x1
movz x0, 0xFFFF         // no se ejecuta
movz x2, 0xABCD         // instrucción en 0x00400008
HLT 0
