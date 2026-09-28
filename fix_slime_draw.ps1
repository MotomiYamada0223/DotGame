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

    // 当たり判定のサイズ設定（画像のサイズをそのまま使うか調整するか）
    // ひとまず画像が400x400と大きめなので、少し縮小するか検討しつつ、
    // 既存の通り 1/6 サイズ程度にするか、一旦そのままの比率で設定
    mfEnemyWidth = 64.0f;
    mfEnemyHeight = 64.0f;
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
        if (isDamaged) { SetDrawBright(255, 100, 100); }

        // 描画位置（中心座標）
        const float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        const float screenY = mvPosition.y;

        // 一枚絵として描画（画像が400x400なので、当たり判定に合わせて縮小して描画する）
        // DrawRotaGraph(x, y, 拡大率, 回転角度, グラフィックハンドル, 透過フラグ)
        float scale = mfEnemyWidth / GetSizeX(); // 当たり判定の幅(64)に合わせる

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
        mMoveSpeed,
        moveDirection
    );
}

void EnemySlime::UpdateStatusByProgress(GameProgress progress)
{
    // 進行度に応じたステータス設定など
}
"@
[System.IO.File]::WriteAllText("Source\EnemySlime.cpp", $content, [System.Text.Encoding]::GetEncoding(932))
