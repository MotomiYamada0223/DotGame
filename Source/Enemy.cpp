#include "Enemy.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"

Enemy::Enemy(VECTOR initPos)
	: Object2D(CharacterGraphPath::Dragon, initPos)
{
	SetTag(Object2D::Enemy2D);

	moveSpeed = 2.0f;
	isDamaged = false;
	damageTimer = 0;
	mnCurrentFrame = 0;
	mnFrameTimer = 0;
}

Enemy::~Enemy()
{
}

void Enemy::Update()
{
	if (isDamaged)
	{
		damageTimer--;
		if (damageTimer <= 0)
		{
			isDamaged = false;
		}
	}


	// アニメーション更新
	mnFrameTimer++;
	if (mnFrameTimer >= FRAME_INTERVAL)
	{
		mnFrameTimer = 0;
		mnCurrentFrame =
			(mnCurrentFrame + 1) % TOTAL_FRAMES;
	}

	Object2D::Update();
}

void Enemy::Draw()
{
	if (mpTexture != nullptr)
	{
		const int srcX = mnCurrentFrame * FRAME_WIDTH;
		int srcY = 256;

		if (isDamaged) { SetDrawBright(255, 100, 100); }

		// プレイヤーと同じ共通関数でスクロール位置を完全に一致させる
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



void Enemy::OnDamaged()
{
	// 攻撃を受けた際に被ダメージ状態にし赤く光るタイマーをセットするため
	isDamaged = true;
	damageTimer = 30; // 30フレーム赤く光る
}


void Enemy::EnemyMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap; // マップの参照を保持

	// 敵の移動処理
	mvPosition.x += moveSpeed;

	// アニメーション更新など
	mnFrameTimer++;
	if (mnFrameTimer >= FRAME_INTERVAL)
	{
		mnFrameTimer = 0;
		mnCurrentFrame = (mnCurrentFrame + 1) % TOTAL_FRAMES;
	}
}

