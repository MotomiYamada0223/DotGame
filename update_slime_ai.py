import sys

# EnemySlime.h
try:
    with open('Source/EnemySlime.h', 'r', encoding='utf-8-sig') as f:
        h_content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.h', 'r', encoding='cp932') as f:
        h_content = f.read()

h_content = h_content.replace(
    'TextureAnimation* mpAnimAttack;\n\nprotected:',
    'TextureAnimation* mpAnimAttack;\n    float mCurrentMoveDirection;\n    int mActionTimer;\n    bool mIsFacingRight;\n\nprotected:'
)

with open('Source/EnemySlime.h', 'w', encoding='utf-8-sig') as f:
    f.write(h_content)


# EnemySlime.cpp
try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        cpp_content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        cpp_content = f.read()

# コンストラクタに初期化追加
cpp_content = cpp_content.replace(
    'UpdateStatusByProgress(GameProgress::Tutorial1);',
    'mCurrentMoveDirection = 1.0f;\n    mActionTimer = 60;\n    mIsFacingRight = true;\n\n    UpdateStatusByProgress(GameProgress::Tutorial1);'
)

# EnemyMoveのランダム移動化
old_move = """void EnemySlime::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    float moveDirection = 1.0f;

    mCharacterPhysics.UpdateMoveAndCollision(
        mvPosition,
        velocityY,
        isGrounded,
        mbIsJumping,
        blockMap,
        mfEnemyWidth,
        mfEnemyHeight,
        gravity,
        mMoveSpeed,
        moveDirection
    );
}"""

new_move = """void EnemySlime::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    
    // ランダム行動タイマー
    mActionTimer--;
    if (mActionTimer <= 0)
    {
        int randAction = GetRand(2); // 0, 1, 2
        if (randAction == 0) mCurrentMoveDirection = 1.0f;
        else if (randAction == 1) mCurrentMoveDirection = -1.0f;
        else mCurrentMoveDirection = 0.0f; // 停止
        
        // 1秒から3秒で次の行動へ
        mActionTimer = 60 + GetRand(120);
    }
    
    // 向きの保存
    if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
    else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;

    mCharacterPhysics.UpdateMoveAndCollision(
        mvPosition,
        velocityY,
        isGrounded,
        mbIsJumping,
        blockMap,
        mfEnemyWidth,
        mfEnemyHeight,
        gravity,
        mMoveSpeed,
        mCurrentMoveDirection
    );
}"""
cpp_content = cpp_content.replace(old_move, new_move)

# Drawの修正 (移動方向に応じたアニメーションと反転)
import re
pattern_draw = r"void EnemySlime::Draw\(\)\s*\{[\s\S]*?Object2D::Draw\(\);\s*\n\s*\}"

new_draw = """void EnemySlime::Draw()
{
    if (mpBlockMap != nullptr)
    {
        if (isDamaged) { SetDrawBright(255, 100, 100); }

        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;
        
        // 現在のモーションを決定
        TextureAnimation* currentAnim = mpAnimIdle; // 基本はIdle
        
        if (mCurrentMoveDirection != 0.0f)
        {
            currentAnim = mpAnimMove; // 移動中ならMove
        }
        
        // 攻撃判定などがあれば mpAnimAttack にするが、今回は移動と待機のみ
        
        if (currentAnim != nullptr)
        {
            currentAnim->SetPosition(VGet(screenX, screenY, 0.0f));
            
            // 右向きならfalse、左向きならtrueで反転させる (画像がデフォで右向きを想定)
            bool turnFlag = !mIsFacingRight;
            currentAnim->Draw(turnFlag);
        }

        if (isDamaged) { SetDrawBright(255, 255, 255); }
    }
    else
    {
        Object2D::Draw();
    }
}"""

match = re.search(pattern_draw, cpp_content)
if match:
    cpp_content = cpp_content[:match.start()] + new_draw + cpp_content[match.end():]
else:
    print("Failed to replace Draw()")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(cpp_content)

