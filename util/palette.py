#!/usr/bin/env python3

colors = [
#    (8, 24, 32),
#    (52, 104, 86),
#    (136, 192, 112),
#    (224, 248, 208),
    (15, 56, 15),
    (48, 98, 48),
    (139, 172, 15),
    (155, 188, 15),
]

def convert(c):
    return (round(c[0] / 8), round(c[1] / 4), round(c[2]/ 8))

for c in colors:
    print(f"{c} -> {convert(c)}")
