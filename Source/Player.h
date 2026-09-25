#pragma once
#include "Object2D.h"
#include <vector>
#include <math.h>
#include "BlockMap.h"
#include "CharacterPhysics.h" // ジャンプとかの当たり判定をしてくれる処理
#include "BlockAction.h"
#include "FallDeathController.h"

#include "UnitStatus.h"

class Player : public Object2D, public UnitStatus
{
public:
	Player(VECTOR initPos);
	virtual ~Player();

	virtual void Update() override;
	virtual void Draw() override;
	virtual void UpdateStatusByProgress(GameProgress progress) override;

	void DebugDraw(); // デバッグ用の描画関数

	// Mapを受け取って位置を更新する処理
	void PlayerMove(BlockMap& blockMap);

	float GetMoveDirection() const { return mfMoveDirection; }
	// 死亡テキストの呼び出し
	void DrawFallDeath();

public:
	bool GetIsDead() const { return isDead; } // 死亡判定かの処理


private:
	CharacterPhysics mCharacterPhysics; // 物理計算用のインスタンス
	FallDeathController mFallDeath; // 落下処理
	float mfMoveDirection;

	// プレイヤーの当たり判定サイズ
	float mfPlayerWidth;
	float mfPlayerHeight;

	// ジャンプ関連
	bool mbIsJumping;
	float velocityY;
	const float gravity = 0.3f; // 元0.5
	const float jumpPower = -16.0f; // 元12
	bool isGrounded; // 地面に接地しているかどうか

	// 移動関連
	const float moveSpeed = 5.0f;
	bool isFacingRight; 

	// 攻撃関連
	bool isAttacking;
	int attackTimer;
	const int attackDuration = 15; 

	// 被ダメージ（衝突）フラグ
	bool isHitDamage;

	// アニメーション関連
	int mCurrentFrame;
	int mFrameTimer;
	static const int FRAME_WIDTH = 128; 
	static const int FRAME_HEIGHT = 128;
	static const int TOTAL_FRAMES = 4; 
	static const int FRAME_INTERVAL = 8; 

	// 死亡処理関連
	struct PlayerFragment
	{
		VECTOR pos;
		VECTOR vel;
		int srcX, srcY;
		int width, height;
	};

	std::vector<PlayerFragment> mFragments;
	VECTOR mSpawnPos;
	int mDeadState;
	BlockMap* mpBlockMap;
	BlockAction mBlockAction;

	bool isDead;
	int deadTimer;
	float alpha;

	void DeadProcess();

	int mHeartFullGraph;
	int mHeartHalfGraph;
	int mHeartEmptyGraph;

private: // 復活処理関係
	void Revive();
	bool mbFallDeath = false; // 死亡判定
};