.text
movz x0, 0x0001
b salto
movz x0, 0x0002  // no se ejecuta
salto:
orr x1, x0, x0   // para verificar que estamos en el lugar correcto
hlt