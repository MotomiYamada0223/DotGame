import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

includes = """#include "EnemySlime.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"
#include "Master.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameManager.h"
#include <cmath>

"""

if "#include" not in content:
    content = includes + content

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

