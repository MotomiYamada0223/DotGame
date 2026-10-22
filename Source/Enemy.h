#pragma once

#include "Object2D.h"
#include "BlockMap.h"
#include "CharacterPhysics.h"

class Enemy : public Object2D
{
public:
	Enemy(const std::string& graphPath, VECTOR initPos);
	virtual ~Enemy();

	virtual void Update() override;
	virtual void Draw() override;

	// 敵の移動処理
	virtual void EnemyMove(BlockMap& blockMap) {}

	// 攻撃を受けた際に呼ばれる処理
	void OnDamaged();

	// 左右の移動制限をさせる関数
	void LimitPositionX(const BlockMap& blockMap, float collisionWidth);

	// 当たり判定サイズの取得
	virtual float GetCollisionWidth() const { return mfEnemyWidth; }
	virtual float GetCollisionHeight() const { return mfEnemyHeight; }
	virtual float GetCollisionOffsetY() const { return 0.0f; }

	// 本体矩形以外の追加攻撃判定（ビーム等）との衝突チェック
	virtual bool CheckAttackCollision(const VECTOR& targetPos, const VECTOR& targetSize) const { return false; }

	// カスタム死亡演出を持つか
	virtual bool HasCustomDeath() const { return false; }
	// プレイヤーとの本体接触判定・被攻撃判定が有効か
	virtual bool IsActiveTarget() const { return true; }

protected:
	// 移動・物理関連
	
	// CharacterPhysics用
	bool mbIsJumping;
	bool isGrounded;
	float velocityY;

	// 重力
	float gravity;

	// 敵の当たり判定サイズ
	float mfEnemyWidth;
	float mfEnemyHeight;


	// ダメージ関連
	bool isDamaged;
	int damageTimer;


	// アニメーション関連
	int mnCurrentFrame;
	int mnFrameTimer;

	static const int FRAME_WIDTH = 256;
	static const int FRAME_HEIGHT = 256;
	static const int TOTAL_FRAMES = 4;
	static const int FRAME_INTERVAL = 13;


	// マップ
	BlockMap* mpBlockMap;


	// 物理計算
	CharacterPhysics mCharacterPhysics;
};