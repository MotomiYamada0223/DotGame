import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        lines = f.readlines()

for i, line in enumerate(lines):
    if "DrawQuadrangle" in line:
        for j in range(max(0, i-25), min(len(lines), i+10)):
            print(f"{j+1}: {lines[j].rstrip()}")
        break
