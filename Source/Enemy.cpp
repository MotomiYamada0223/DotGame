#include "Enemy.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"
#include "BlockMap.h"


Enemy::Enemy(VECTOR initPos)
	: Object2D(CharacterGraphPath::Dragon, initPos)
	, mpBlockMap(nullptr)
{
	SetTag(Object2D::Enemy2D);


	// 移動関連
	moveSpeed = 2.0f;

	// CharacterPhysics用
	mbIsJumping = false;
	isGrounded = false;
	velocityY = 0.0f;

	// 重力
	// Playerと同じ値を使用する
	gravity = 0.5f;


	// 敵の当たり判定サイズ
	mfEnemyWidth = GetSizeX() / 6.0f;
	mfEnemyHeight = GetSizeY() / 6.0f;

	// ダメージ関連
	isDamaged = false;
	damageTimer = 0;


	// アニメーション関連
	mnCurrentFrame = 0;
	mnFrameTimer = 0;
}


Enemy::~Enemy()
{
}


// Update
void Enemy::Update()
{
	// ダメージ状態更新
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


	// Object2Dの更新
	Object2D::Update();
}


void Enemy::Draw()
{
	if (mpTexture != nullptr)
	{
		// アニメーション画像のX座標
		const int srcX =
			mnCurrentFrame * FRAME_WIDTH;

		// 右向きの画像
		int srcY = 256;


		// ダメージ中は赤くする
		if (isDamaged)
		{
			SetDrawBright(255, 100, 100);
		}

		// スクリーン座標へ変換
		const int screenX =
			static_cast<int>(
				ConvertToScreenX(
					mvPosition.x,
					mpBlockMap
				)
				- FRAME_WIDTH / 2
				);

		const int screenY =
			static_cast<int>(
				mvPosition.y
				- FRAME_HEIGHT / 2
				);

		// 敵を描画
		DrawRectGraph(
			screenX,
			screenY,
			srcX,
			srcY,
			FRAME_WIDTH,
			FRAME_HEIGHT,
			mpTexture->GetHandle(),
			true
		);


		// 色を元に戻す
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


// EnemyMove
void Enemy::EnemyMove(BlockMap& blockMap)
{
	// マップの参照を保持
	mpBlockMap = &blockMap;

	// 敵の移動方向
	float moveDirection = 1.0f;


	// BlockMapとの当たり判定付き移動
	mCharacterPhysics.UpdateMoveAndCollision(
		mvPosition,
		velocityY,
		isGrounded,
		mbIsJumping,
		blockMap,
		mfEnemyWidth,
		mfEnemyHeight,
		gravity,
		moveSpeed,
		moveDirection
	);
}

void Enemy::OnDamaged()
{
	// 攻撃を受けた
	isDamaged = true;

	// 30フレーム赤くする
	damageTimer = 30;
}