#pragma once
#include "Object2D.h"
#include <vector>
#include <math.h>
#include "BlockMap.h"
#include "CharacterPhysics.h" // ジャンプとかの当たり判定をしてくれる処理
#include "BlockAction.h"
#include "FallDeathController.h"
#include "PlayerAnimState.h"
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

	void PlayerMove(BlockMap& blockMap); // Mapを受け取って位置を更新する処理

	void DrawFallDeath(); // 死亡テキストの呼び出し
	void DrawLife(); // 体力はあとの描画


public: // アクセサ
	float GetMoveDirection() const { return mfMoveDirection; }
	bool GetIsDead() const { return isDead; } // 死亡判定かの処理
	float GetCurrentSpeed() const { return mfCurrentSpeed; }
	float GetVelocityY() const { return velocityY; } // ジャンプ中かどうかの判定用に追加
	void SetVelocityY(float vy) { velocityY = vy; }

	// 動く床などのオブジェクトに乗った際に接地状態を強制するためのSetter
	void SetForceGrounded() { 
		mForceGroundedThisFrame = true;
	}  // 頭をぶつけた時の落下処理用に追加 // 現在の速さ

private:
	// インスタンスで持っているもの
	CharacterPhysics mCharacterPhysics; // 物理計算用のインスタンス
	FallDeathController mFallDeath; // 落下処理
	PlayerAnimState mPlayerState;
	BlockAction mBlockAction;

	// ポインタで持っているもの
	BlockMap* mpBlockMap;


	float mfMoveDirection;

	// プレイヤーの当たり判定サイズ
	float mfPlayerWidth;
	float mfPlayerHeight;

	// ジャンプ関連
	bool mbIsJumping;
	float velocityY;
	const float gravity = 0.3f; // 元0.5
	const float jumpPower = -15.0f; // 元12
	bool isGrounded; // 地面に接地しているかどうか
	bool mForceGroundedThisFrame; // 外部オブジェクトにより強制接地させるフラグ

	// 移動関連
	const float moveSpeed = 5.0f;
	float mfCurrentSpeed = moveSpeed; // 合計の今の速さ
	bool isFacingRight; // 右向きかどうか

	// 攻撃関連
	bool isAttacking;
	int attackTimer;
	std::vector<Object2D*> mHitEnemies;
	const int attackDuration = 15;


	// 被ダメージ（衝突）フラグ
	bool isHitDamage;
	int mInvincibleTimer;


	// 死亡処理関連
	// ブリンク関連
	bool isBlinking;
	int blinkTimer;
	int blinkCooldownTimer;
	const int blinkDuration = 12; // 約0.2秒
	const float blinkSpeed = 15.0f;
	float blinkDirection;

	// ブリンクの時に必要な処理
	struct BlinkAfterimage
	{
		VECTOR pos;
		bool facingRight;
		float alpha;
	};
	std::vector<BlinkAfterimage> mAfterimages;

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