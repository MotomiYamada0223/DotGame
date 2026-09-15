#include "Enemy.h"
#include "DxLib.h"
#include "Texture.h"
#include "GameConstants.h"

Enemy::Enemy(VECTOR initPos)
	: Object2D(CharacterGraphPath::Skeleton, initPos)
{
	SetTag(Object2D::Enemy2D);

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
	
	mvPosition.x += moveSpeed;

	if (isDamaged)
	{
		damageTimer--;
		if (damageTimer <= 0)
		{
			isDamaged = false;
		}
	}

	Object2D::Update();
}

void Enemy::Draw()
{
	if (isDamaged)
	{
		SetDrawBright(255, 100, 100);
	}

	Object2D::Draw();

	if (isDamaged)
	{
		SetDrawBright(255, 255, 255);
	}
}

void Enemy::OnDamaged()
{
	isDamaged = true;
	damageTimer = 30; // 30フレーム赤く光る
	
}
