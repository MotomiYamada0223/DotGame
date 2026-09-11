#include "Enemy.h"
#include "DxLib.h"
#include "Master.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Player.h"
#include "Texture.h"

Enemy::Enemy(VECTOR initPos)
	: Object2D("Resource/Image/SampleSkeleton.png", initPos) 
	: Object2D("Resource/Image/enemy_dragon_move.png", initPos)
{
	SetTag(Object2D::Enemy2D);
	mnCurrentFrame = 0;
	mnFrameTimer = 0;
	mvPlayerDirection = VGet(0.0f, 0.0f, 0.0f);
}

	moveSpeed = 2.0f;
	isDamaged = false;
	damageTimer = 0;
}

Enemy::~Enemy()
{

}

void Enemy::Update()
{
	// 画面左から出現するので、右へ移動させる
	bool moved = false;

	auto pObj = Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->GetObject2DByTag(Object2D::BattlePlayer2D);
	if (pObj != nullptr)
	{
		Player* pPlayer = dynamic_cast<Player*>(pObj);
		if (pPlayer == nullptr)
		{
			return;
		}

		VECTOR playerToEnemy = VSub(pPlayer->GetPosition(), mvPosition);
		float distance = VSize(playerToEnemy);

		if (distance <= SearchRange)
		{
			mvPlayerDirection.x = (playerToEnemy.x > 0.0f) ? 1.0f : -1.0f;
			mvPosition.x += MoveSpeed * mvPlayerDirection.x;
			moved = true;
		}
		else
		{
			mvPlayerDirection.x = 0.0f;
			moved = false;
		}
	
	mvPosition.x += moveSpeed;
	}

	if (isDamaged)
	if (moved)
	{
		damageTimer--;
		if (damageTimer <= 0)
		mnFrameTimer++;

		if (mnFrameTimer >= FRAME_INTERVAL)
		{
			isDamaged = false;
			mnFrameTimer = 0;
			mnCurrentFrame = (mnCurrentFrame + 1) % TOTAL_FRAMES;
		}
	}
	else
	{
		mnCurrentFrame = 0;
		mnFrameTimer = 0;
	}

	Object2D::Update();
}

void Enemy::Draw()
{
	if (isDamaged)
	if (mpTexture != nullptr)
	{
		SetDrawBright(255, 100, 100);
	}
		const int srcX = mnCurrentFrame * FRAME_WIDTH;

	Object2D::Draw();
		int srcY = 101;

	if (isDamaged)
		if (mvPlayerDirection.x > 0.0f)
	{
		SetDrawBright(255, 255, 255);
			srcY = 354;
	}
		if (mvPlayerDirection.x < 0.0f)
		{
			srcY = 101;
}

void Enemy::OnDamaged()
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

	}
	else
{
	isDamaged = true;
	damageTimer = 30; // 30フレーム赤く光る
		Object2D::Draw();
	}
	
}
