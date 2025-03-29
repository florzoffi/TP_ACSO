.text
MOVZ X5, 0
MOVZ X6, 10
CBZ X5, salto
MOVZ X7, 111
salto:
CBZ X6, fin
MOVZ X8, 888
fin:
MOVZ X9, 999
HLT 0
