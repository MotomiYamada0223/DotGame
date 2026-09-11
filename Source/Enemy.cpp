#include "Enemy.h"
#include "Master.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Player.h"
#include "Texture.h"

Enemy::Enemy(VECTOR initPos)
	: Object2D("Resource/Image/enemy_dragon_move.png", initPos)
{
	SetTag(Object2D::Enemy2D);
	mnCurrentFrame = 0;
	mnFrameTimer = 0;
	mvPlayerDirection = VGet(0.0f, 0.0f, 0.0f);
}


Enemy::~Enemy()
{

}

void Enemy::Update()
{
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

	}

	if (moved)
	{
		mnFrameTimer++;

		if (mnFrameTimer >= FRAME_INTERVAL)
		{
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
	if (mpTexture != nullptr)
	{
		const int srcX = mnCurrentFrame * FRAME_WIDTH;

		int srcY = 101;

		if (mvPlayerDirection.x > 0.0f)
		{
			srcY = 354;
		}
		if (mvPlayerDirection.x < 0.0f)
		{
			srcY = 101;
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

	}
	else
	{
		Object2D::Draw();
	}

}
