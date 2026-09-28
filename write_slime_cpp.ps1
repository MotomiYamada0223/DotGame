$content = @"
#include "EnemySlime.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"

EnemySlime::EnemySlime(VECTOR initPos)
    : Enemy(initPos) // 基底コンストラクタ呼び出し
{
    // 画像はスライム専用のものを使用
    mpTexture = Master::mpGameManager->GetResourceManager()->GetTexture("Resource/Image/SampleSlime.png");

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
    // ダメージ更新など共通のUpdateを呼ぶ
    Enemy::Update();
}

void EnemySlime::Draw()
{
    if (mpTexture != nullptr)
    {
        const int srcX = mnCurrentFrame * FRAME_WIDTH;
        int srcY = 256;

        if (isDamaged) { SetDrawBright(255, 100, 100); }

        const int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap) - FRAME_WIDTH / 2);
        const int screenY = static_cast<int>(mvPosition.y - FRAME_HEIGHT / 2);

        DrawRectGraph(screenX, screenY, srcX, srcY, FRAME_WIDTH, FRAME_HEIGHT, mpTexture->GetHandle(), true);

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
        mMoveSpeed, // スライム専用の速度を渡す
        moveDirection
    );
}

void EnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    // 進行度に応じたステータス設定など
}
"@
[System.IO.File]::WriteAllText("Source\EnemySlime.cpp", $content, [System.Text.Encoding]::GetEncoding(932))
