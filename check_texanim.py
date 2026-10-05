import sys

try:
    with open('Source/TextureAnimation.h', 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/TextureAnimation.h', 'r', encoding='cp932') as f:
        lines = f.readlines()

for i, line in enumerate(lines):
    if "GetCurrentFrame" in line:
        for j in range(max(0, i-2), min(len(lines), i+3)):
            print(f"{j+1}: {lines[j].rstrip()}")
        break
