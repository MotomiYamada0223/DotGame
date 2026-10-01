import sys

try:
    with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/Player.cpp', 'r', encoding='cp932') as f:
        lines = f.readlines()

for i, line in enumerate(lines):
    if "if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))" in line:
        print(f"Found attack at line {i+1}")
        for j in range(i-2, i+30):
            if j < len(lines):
                print(f"{j+1}: {lines[j].rstrip()}")
        break
