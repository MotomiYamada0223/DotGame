import sys

def modify():
    try:
        with open('Source/TestScene.cpp', 'r', encoding='utf-8-sig') as f:
            content = f.read()
    except UnicodeDecodeError:
        with open('Source/TestScene.cpp', 'r', encoding='cp932') as f:
            content = f.read()

    # #include "NeedleTrap.h" を追加
    if '#include "NeedleTrap.h"' not in content:
        content = content.replace('#include "TestScene.h"', '#include "TestScene.h"\n#include "NeedleTrap.h"')

    # Initialize内にトラップを配置
    if "NeedleTrap" not in content[content.find("TestScene::Initialize"):]:
        old_init = """	// G̐
	if (mpPlayer != nullptr)
	{
		new EnemySlime(VGet(Utility::SCREEN_WIDTH / 2.0f + 1000.0f, 600.0f, 0.0f));
	}"""
        new_init = """	// G̐
	if (mpPlayer != nullptr)
	{
		new EnemySlime(VGet(Utility::SCREEN_WIDTH / 2.0f + 1000.0f, 600.0f, 0.0f));
		
		// 針トラップをテスト配置
		new NeedleTrap(VGet(Utility::SCREEN_WIDTH / 2.0f + 500.0f, 600.0f, 0.0f), TrapType::PopUp);
		new NeedleTrap(VGet(Utility::SCREEN_WIDTH / 2.0f + 800.0f, 600.0f, 0.0f), TrapType::AntiJump);
	}"""
        content = content.replace(old_init, new_init)

    with open('Source/TestScene.cpp', 'w', encoding='utf-8-sig') as f:
        f.write(content)

modify()
