import sys
import re

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# 必要なインクルードの追加
if '#include "Master.h"' not in content:
    content = content.replace('#include "EnemySlime.h"', '#include "EnemySlime.h"\n#include "Master.h"\n#include "ObjectManager.h"')
elif '#include "ObjectManager.h"' not in content:
    content = content.replace('#include "Master.h"', '#include "Master.h"\n#include "ObjectManager.h"')

# コンストラクタで初期化
content = content.replace("mIsFacingRight = true;", "mIsFacingRight = true;\n    mIsChasing = false;")

# EnemyMoveメソッドの置き換え
old_move_pattern = r"void EnemySlime::EnemyMove\(BlockMap& blockMap\)\s*\{[\s\S]*?mCharacterPhysics\.UpdateMoveAndCollision\("
new_move = """void EnemySlime::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    
    // プレイヤーの取得
    Object2D* playerObj = Master::mpObjectManager->GetObject2DByTag(Object2D::Tag::Player2D);
    mIsChasing = false;
    
    if (playerObj != nullptr)
    {
        float playerX = playerObj->GetPosition().x;
        float playerY = playerObj->GetPosition().y;
        float enemyX = mvPosition.x;
        float enemyY = mvPosition.y;
        
        // 視界のY幅 (上下100以内)
        if (std::abs(playerY - enemyY) < 100.0f)
        {
            if (mIsFacingRight)
            {
                // 右向き：前方に200ピクセル
                if (playerX > enemyX && (playerX - enemyX) <= 200.0f)
                {
                    mIsChasing = true;
                    mCurrentMoveDirection = 1.0f; // 右へ追いかける
                }
            }
            else
            {
                // 左向き：前方に200ピクセル
                if (playerX < enemyX && (enemyX - playerX) <= 200.0f)
                {
                    mIsChasing = true;
                    mCurrentMoveDirection = -1.0f; // 左へ追いかける
                }
            }
        }
    }
    
    // 追跡中でない場合はランダム徘徊
    if (!mIsChasing)
    {
        mActionTimer--;
        if (mActionTimer <= 0)
        {
            int randAction = GetRand(2); // 0, 1, 2
            if (randAction == 0) mCurrentMoveDirection = 1.0f;
            else if (randAction == 1) mCurrentMoveDirection = -1.0f;
            else mCurrentMoveDirection = 0.0f; // 停止
            
            int loopCount = 1 + GetRand(2); // 1, 2, 3
            mActionTimer = loopCount * 48;
        }
        
        // 向きの保存
        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }
    else
    {
        // 追跡中も向きを更新（プレイヤーを追い越した時のため）
        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }

    mCharacterPhysics.UpdateMoveAndCollision("""
match = re.search(old_move_pattern, content)
if match:
    content = content[:match.start()] + new_move + content[match.end():]
else:
    print("Failed to replace EnemyMove")

# Drawメソッドの置き換え (デバッグ表示追加)
old_draw_pattern = r"void EnemySlime::Draw\(\)\s*\{\s*if \(mpBlockMap != nullptr\)\s*\{"
new_draw = """void EnemySlime::Draw()
{
    if (mpBlockMap != nullptr)
    {
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;
        
        // --- 視界のデバッグ表示 ---
        int visionLeft, visionRight, visionTop, visionBottom;
        visionTop = static_cast<int>(screenY - 100.0f);
        visionBottom = static_cast<int>(screenY + 100.0f);
        
        if (mIsFacingRight)
        {
            visionLeft = static_cast<int>(screenX);
            visionRight = static_cast<int>(screenX + 200.0f);
        }
        else
        {
            visionLeft = static_cast<int>(screenX - 200.0f);
            visionRight = static_cast<int>(screenX);
        }
        
        // 追跡中は赤、待機中は黄色
        int color = mIsChasing ? GetColor(255, 0, 0) : GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(visionLeft, visionTop, visionRight, visionBottom, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // ------------------------"""
match = re.search(old_draw_pattern, content)
if match:
    content = content[:match.start()] + new_draw + content[match.end():]
else:
    print("Failed to replace Draw")

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)
