import sys
import re

def modify():
    try:
        with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
            content = f.read()
    except UnicodeDecodeError:
        with open('Source/Player.cpp', 'r', encoding='cp932') as f:
            content = f.read()

    # #include "NeedleTrap.h" を追加
    if '#include "NeedleTrap.h"' not in content:
        content = content.replace('#include "BossEnemyDragon.h"', '#include "BossEnemyDragon.h"\n#include "NeedleTrap.h"')

    # 攻撃処理で NeedleTrap を弾く
    old_attack = """					if (hitEnemy == enemy)
					{
						alreadyHit = true;
						break;
					}
				}
				if (!alreadyHit)
				{
					enemy->OnDamaged();"""
    
    new_attack = """					if (hitEnemy == enemy)
					{
						alreadyHit = true;
						break;
					}
				}
				
				// 針トラップには攻撃無効
				NeedleTrap* trap = dynamic_cast<NeedleTrap*>(enemy);
				if (trap != nullptr)
				{
					continue;
				}

				if (!alreadyHit)
				{
					enemy->OnDamaged();"""
    
    if "NeedleTrap* trap" not in content:
        content = content.replace(old_attack, new_attack)
        
    with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
        f.write(content)

modify()
