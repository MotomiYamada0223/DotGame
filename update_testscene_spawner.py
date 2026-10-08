import sys

with open('Source/TestScene.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# include
if '#include "FloorSpawner.h"' not in content:
    content = content.replace('#include "MovingFloor.h"', '#include "MovingFloor.h"\n#include "FloorSpawner.h"')

# new
spawner_code = """
	// 上から下へ無限に降ってくる足場のスポナーを配置
	// 始点: 6300, -100
	// 速度: Y軸に3.0
	// 間隔: 5秒 = 300フレーム (60fps想定)
	new FloorSpawner(
		VGet(6300.0f, -100.0f, 0.0f),
		VGet(0.0f, 3.0f, 0.0f),
		300,
		mpPlayer,
		&mBlockMap
	);
"""
# 例2のコメントの下あたりに追加する
content = content.replace('	//MovingFloor* floor2 = new MovingFloor(', spawner_code + '\n	//MovingFloor* floor2 = new MovingFloor(')

with open('Source/TestScene.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

