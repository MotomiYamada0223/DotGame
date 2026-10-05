import sys

# EnemySlime.h
try:
    with open('Source/EnemySlime.h', 'r', encoding='utf-8-sig') as f:
        h_content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.h', 'r', encoding='cp932') as f:
        h_content = f.read()

if '#include "TextureAnimation.h"' not in h_content:
    h_content = h_content.replace('#include "UnitStatus.h"', '#include "UnitStatus.h"\n#include "TextureAnimation.h"')
    
    # メンバ変数の追加
    h_content = h_content.replace('protected:', 'private:\n    TextureAnimation* mpAnimIdle;\n    TextureAnimation* mpAnimMove;\n    TextureAnimation* mpAnimAttack;\n\nprotected:')

with open('Source/EnemySlime.h', 'w', encoding='utf-8-sig') as f:
    f.write(h_content)


# EnemySlime.cpp
try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        cpp_content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        cpp_content = f.read()

# コンストラクタ
old_init = """    mfEnemyWidth = 64.0f;
    mfEnemyHeight = 64.0f;"""
new_init = """    mfEnemyWidth = 64.0f;
    mfEnemyHeight = 64.0f;

    mpAnimIdle = new TextureAnimation(initPos, "Resource/Image/slime_idle.png", 6, 6, 1, 10);
    mpAnimMove = new TextureAnimation(initPos, "Resource/Image/slime_move.png", 6, 6, 1, 10);
    mpAnimAttack = new TextureAnimation(initPos, "Resource/Image/slime_attack.png", 7, 7, 1, 10);"""
cpp_content = cpp_content.replace(old_init, new_init)

# デストラクタ
old_dest = """EnemySlime::~EnemySlime()
{
}"""
new_dest = """EnemySlime::~EnemySlime()
{
    if (mpAnimIdle) delete mpAnimIdle;
    if (mpAnimMove) delete mpAnimMove;
    if (mpAnimAttack) delete mpAnimAttack;
}"""
cpp_content = cpp_content.replace(old_dest, new_dest)

# Update
old_update = """void EnemySlime::Update()
{
    Enemy::Update();
}"""
new_update = """void EnemySlime::Update()
{
    Enemy::Update();
    if (mpAnimIdle) mpAnimIdle->Update();
    if (mpAnimMove) mpAnimMove->Update();
    if (mpAnimAttack) mpAnimAttack->Update();
}"""
cpp_content = cpp_content.replace(old_update, new_update)

# Draw
old_draw = """void EnemySlime::Draw()
{
    if (mpTexture != nullptr)
    {
        if (isDamaged) { SetDrawBright(255, 100, 100); }

        // `ʒuiSWj
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;

        // ꖇGƂĕ`i摜400x400Ȃ̂ŁA蔻ɍ킹ďkĕ`悷j
        // DrawRotaGraph(x, y, g嗦, ]px, OtBbNnh, ߃tO)
        float scale = mfEnemyWidth / GetSizeX(); // 蔻̕(64)ɍ킹

        DrawRotaGraph(
            static_cast<int>(screenX),
            static_cast<int>(screenY),
            scale,
            0.0,
            mpTexture->GetHandle(),
            TRUE
        );

        if (isDamaged) { SetDrawBright(255, 255, 255); }
    }
    else
    {
        Object2D::Draw();
    }
}"""
new_draw = """void EnemySlime::Draw()
{
    if (mpBlockMap != nullptr && mpAnimMove != nullptr)
    {
        if (isDamaged) { SetDrawBright(255, 100, 100); }

        // スクロール対応のため、スクリーン座標を求めてアニメーションクラスにセットする
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;
        
        mpAnimMove->SetPosition(VGet(screenX, screenY, 0.0f));
        mpAnimMove->Draw(); // とりあえず現在は常にMoveモーションを描画

        if (isDamaged) { SetDrawBright(255, 255, 255); }
    }
    else
    {
        Object2D::Draw();
    }
}"""

import re
pattern_draw = r"void EnemySlime::Draw\(\)\s*\{[\s\S]*?Object2D::Draw\(\);\s*\n\s*\}"
match = re.search(pattern_draw, cpp_content)
if match:
    cpp_content = cpp_content[:match.start()] + new_draw + cpp_content[match.end():]
else:
    print("Draw replacement failed.")


with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(cpp_content)

