import sys
import glob
import os

files = glob.glob("Source/*Enemy*.cpp")

for f in files:
    with open(f, 'r', encoding='cp932', errors='ignore') as file:
        content = file.read()
    
    if os.path.basename(f) == "Enemy.cpp":
        continue
    
    # Replace Enemy(initPos) with Enemy(CharacterGraphPath::Dragon, initPos)
    if "Enemy(initPos)" in content:
        if os.path.basename(f) == "EnemySlime.cpp":
            content = content.replace("Enemy(initPos)", "Enemy(\"Resource/Image/SampleSlime.png\", initPos)")
        else:
            content = content.replace("Enemy(initPos)", "Enemy(CharacterGraphPath::Dragon, initPos)")
    
    # EnemySlime.cpp specific fixes
    if os.path.basename(f) == "EnemySlime.cpp":
        if "#include \"Master.h\"" not in content:
            content = content.replace("#include \"EnemySlime.h\"", "#include \"EnemySlime.h\"\n#include \"Master.h\"\n#include \"GameManager.h\"\n#include \"ResourceManager.h\"")

    with open(f, 'w', encoding='cp932') as file:
        file.write(content)

print("Done")
