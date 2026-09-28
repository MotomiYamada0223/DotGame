$content = @"
#include "EnemySlime.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"

EnemySlime::EnemySlime(VECTOR initPos)
    : Enemy("Resource/Image/SampleSlime.png", initPos)
{
    mMoveSpeed = 2.0f; // スライム専用の移動速度

    // アニメーション設定などの上書き
    mfEnemyWidth = GetSizeX() / 6.0f;
    mfEnemyHeight = GetSizeY() / 6.0f;
}

EnemySlime::~EnemySlime()
{
}

void EnemySlime::Update()
{
    Enemy::Update();
}

void EnemySlime::Draw()
{
    if (mpTexture != nullptr)
    {
        const int srcX = mnCurrentFrame * 256; // FRAME_WIDTHは基底から消えたので手動指定
        int srcY = 256;

        if (isDamaged) { SetDrawBright(255, 100, 100); }

        const int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap) - 256 / 2);
        const int screenY = static_cast<int>(mvPosition.y - 256 / 2);

        DrawRectGraph(screenX, screenY, srcX, srcY, 256, 256, mpTexture->GetHandle(), true);

        if (isDamaged) { SetDrawBright(255, 255, 255); }
    }
    else
    {
        Object2D::Draw();
    }
}

void EnemySlime::EnemyMove(BlockMap& blockMap)
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
        mMoveSpeed, // スライム専用の速度
        moveDirection
    );
}

void EnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    // 進行度に応じたステータス設定など
}
"@
[System.IO.File]::WriteAllText("Source\EnemySlime.cpp", $content, [System.Text.Encoding]::GetEncoding(932))
