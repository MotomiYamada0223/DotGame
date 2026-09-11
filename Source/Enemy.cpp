#include "Enemy.h"
#include "DxLib.h"
#include "Texture.h"

Enemy::Enemy(VECTOR initPos)
	: Object2D("Resource/Image/enemy_dragon_move.png", initPos)
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
	// 画面左から出現するので、右へ移動させる

	mvPosition.x += moveSpeed;

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

		int srcY = 335;

		if (isDamaged)
		{
			SetDrawBright(255, 100, 100);
		}

		DrawRectGraph(
			static_cast<int>(mvPosition.x - FRAME_WIDTH / 2),
			static_cast<int>(mvPosition.y - FRAME_HEIGHT / 2),
			srcX,
			srcY,
			FRAME_WIDTH,
			FRAME_HEIGHT,
			mpTexture->GetHandle(),
			true
		);

		if (isDamaged)
		{
			SetDrawBright(255, 255, 255);
		}

	}
	else
	{
		Object2D::Draw();
	}

}

void Enemy::OnDamaged()
{
	isDamaged = true;
	damageTimer = 30; // 30フレーム赤く光る

}
