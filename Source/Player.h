#pragma once
#include "Object2D.h"
#include <vector>
#include <math.h>
#include "BlockMap.h"
#include "CharacterPhysics.h" // ジャンプとかの当たり判定をしてくれる処理
#include "BlockAction.h"
#include "DeathController.h"
#include "PlayerAnimState.h"
#include "UnitStatus.h"
#include "DeathReason.h"
#include "PlayerActionState.h"
#include "EnemySlime.h"
#include "EnemySkeleton.h"

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

	void DrawFallDeath(); // 死亡テキストの呼び出し（天使カットイン＆GAME OVER）
	void DrawLife(); // 体力はあとの描画
	void DrawFront(); // Sキー表側落下時の手前描画用

	// 死亡フェーズ取得（GameSceneから天使演出制御に使用）
	bool IsDeathCutInActive() const { return isDead && mDeath.IsAngelCutInStage(); }
	enum class StageFallMode
	{
		None,
		Back,   // Wキー長押し：ステージ裏側（マップ奥）へ落下
		Front   // Sキー長押し：ステージ表側（マップ手前）へ落下
	};

	StageFallMode GetStageFallMode() const { return mFallMode; }
	bool IsFallingThrough() const { return mFallMode != StageFallMode::None; }
	bool IsFallingFront() const { return mFallMode == StageFallMode::Front; }


public: // アクセサ
	float GetMoveDirection() const { return mfMoveDirection; }
	bool GetIsDead() const { return isDead; } // 死亡判定かの処理
	float GetCurrentSpeed() const { return mfCurrentSpeed; }
	float GetVelocityY() const { return velocityY; } // ジャンプ中かどうかの判定用に追加

	// 動く床などのオブジェクトに乗った際に接地状態を強制するためのSetter
	void SetForceGrounded()
	{
		mForceGroundedThisFrame = true;
	}

	// 頭をぶつけた時の落下処理用に追加 // 現在の速さ
	void SetVelocityY(float vy) { velocityY = vy; }  // 頭をぶつけた時の落下処理用に追加 // 現在の速さ
	// 死亡理由を取得
	DeathReason::DeathType GetDeathReason() const { return mDeathReason; }

	// 各シーンからプレイヤーの行動をセット取得するもの
	const PlayerActionState& GetActionState() { return mActionState; }


	bool IsBlinking() const { return isBlinking; } // ブリンク中かどうかの判定用

	// ボス演出・自動歩行関連
	void StartAutoWalk(float targetX);
	void StopAutoWalk();
	bool IsAutoWalking() const { return mbAutoWalking; }
	bool IsAutoWalkArrived() const { return mbAutoWalkArrived; }
	bool IsTouchingBossArea() const { return mbTouchingBossArea; }

	// キングスライム出現演出用回避
	void TriggerCinematicEvasion();
	bool HasEvadedBossSpawn() const { return mbEvadedBossSpawn; }
	bool IsCinematicEvading() const { return mbIsEvading; }
	float GetPlayerWidth() const { return mfPlayerWidth; }
	float GetPlayerHeight() const { return mfPlayerHeight; }
	void SetDeathReason(DeathReason::DeathType type) { mDeathReason = type; }
	void TakeBulletDamage(int damage);
	void ResetBossIntroState();
	void EndBossIntroLock();
	void SetSwordReadyStance(bool active) { mbSwordReadyStance = active; }
	bool IsSwordReadyStance() const { return mbSwordReadyStance; }

	// 接地中かの判定
	bool GetIsGrounded() const { return isGrounded; }
	// ジャンプ中かの判定
	bool GetIsJumping() const { return mbIsJumping; }

	void SetInHotSpring(bool inWater) { mbInHotSpring = inWater; }
	bool GetInHotSpring() const { return mbInHotSpring; }
	void SetHotSpringHeat(float heat) { mHotSpringHeat = heat; }
	float GetHotSpringHeat() const { return mHotSpringHeat; }


	void Heal(int amount)
	{
		if (!isDead && mHp > 0)
		{
			mHp += amount;
			if (mHp > mMaxHp) mHp = mMaxHp;
		}
	}

	void HealHalfHeart()
	{
		if (!isDead && mHp > 0 && mMaxHp > 0)
		{
			int step = (mMaxHp + 5) / 6;
			if (step < 1) step = 1;
			Heal(step);
		}
	}


private:
	// ステージ裏・表落下処理用
	StageFallMode mFallMode;
	int mWKeyHoldTimer;
	int mSKeyHoldTimer;
	void DrawPlayerModel(); // 共通のプレイヤー描画処理

	// インスタンスで持ってる
	CharacterPhysics mCharacterPhysics; // 物理計算用のインスタンス
	DeathController mDeath; // 落下処理
	PlayerAnimState mPlayerState;
	BlockAction mBlockAction;
	DeathReason::DeathType mDeathReason;
	PlayerActionState mActionState;


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
	const float blinkSpeed = 30.0f;
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
	bool mbDeath = false; // 死亡判定

	// ========== 死亡演出フェーズ管理 ==========
	// 0: 通常
	// 1: 天使カットイン（mDeath が担当）
	// 2: 飛び散り（mFragments を DeadProcess で更新）
	// 3: GAME OVER 表示
	int mDeathPhase = 0;
	int mGameOverFadeTimer = 0; // GAME OVERフェードタイマー
	int mGameOverMenuIndex = 0; // GAME OVERメニュー選択（0=リトライ, 1=ステージ選択）

	// ボス演出・自動歩行関連
	bool mbAutoWalking = false;
	bool mbAutoWalkArrived = false;
	float mAutoWalkTargetX = 0.0f;
	bool mbTouchingBossArea = false;

	// キングスライム出現演出・回避関連
	bool mbIsEvading = false;
	bool mbEvadedBossSpawn = false;
	int mEvasionTimer = 0;
	bool mbSwordReadyStance = false;


	bool mbInHotSpring = false;
	float mHotSpringHeat = 0.0f;
	bool mbSpaceWasPressed = false;

	void BeginDeath();        // 死亡処理の共通化
	void StartDeathScatter(); // 天使カットイン後の飛び散り開始
	void UpdateGameOver();    // GAME OVER中の入力処理
	void DrawGameOver();      // GAME OVER画面の描画
	DeathController::AngelMotion GetAngelMotionForDeathType(DeathReason::DeathType type) const;
};