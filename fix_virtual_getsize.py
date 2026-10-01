import sys

try:
    with open('Source/Object2D.h', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/Object2D.h', 'r', encoding='cp932') as f:
        content = f.read()

content = content.replace("int GetSizeX();", "virtual int GetSizeX();")
content = content.replace("int GetSizeY();", "virtual int GetSizeY();")

with open('Source/Object2D.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)
