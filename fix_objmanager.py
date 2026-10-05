import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# 必要なインクルードを追加
includes = """#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h\""""

if '#include "GameManager.h"' not in content:
    content = content.replace('#include "ObjectManager.h"', includes)

# ObjectManagerの取得方法を修正
old_get = "Object2D* playerObj = Master::mpObjectManager->GetObject2DByTag(Object2D::Tag::Player2D);"
new_get = """ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
    Object2D* playerObj = objManager->GetObject2DByTag(Object2D::Tag::Player2D);"""

content = content.replace(old_get, new_get)

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)
