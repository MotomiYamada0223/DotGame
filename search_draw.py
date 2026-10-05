import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        print("File read successfully")
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        print("File read successfully (cp932)")
        lines = f.readlines()

for i, line in enumerate(lines):
    if "Draw" in line:
        print(f"Line {i+1}: {line.strip()}")
