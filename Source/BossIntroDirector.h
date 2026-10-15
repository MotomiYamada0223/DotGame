#pragma once

#include "BlockMap.h"

class Player;
class BossEnemySlime;

class BossIntroDirector
{
public:
	enum class State
	{
		Inactive,          // 演出開始前
		Walking,           // フェードイン＆中央へ歩行中
		WaitBeforeWarning, // 中央到着後の待機
		Warning,           // 警告表示中＆Eキー回避受付（QTE）
		WarningFadeOut,    // 警告フェードアウト
		BossFalling,       // キングスライム急降下中
		BossLanding,       // 着地・逆再生中
		IntroFinished,     // 出現演出完了
		Ended              // 演出終了
	};

	BossIntroDirector();
	~BossIntroDirector();

	void Update(Player* player, BlockMap& blockMap);
	void Draw() const;

	State GetState() const { return mState; }
	bool IsActive() const { return mState != State::Inactive && mState != State::Ended; }
	bool HasPlayerEvaded() const { return mHasEvaded; }
	float GetWarningX() const { return mWarningX; }
	float GetGroundY() const { return mGroundY; }
	BossEnemySlime* GetBossSlime() const { return mpBossSlime; }

	void EndIntro();
	void Reset();

private:
	void DrawWarning(float screenX, float screenY, float fadeAlphaRatio) const;

private:
	State mState;
	int mFadeTimer;             // 黒帯フェード用タイマー
	int mStateTimer;            // 各ステートの待機・演出用タイマー
	float mCurrentTargetX;      // ボスエリア中央X座標
	float mWarningX;            // 警告X座標（プレイヤー座標）
	float mGroundY;             // 警告地面Y座標（足元）
	BlockMap* mpBlockMap; // 画面座標変換用
	bool mHasEvaded;            // 回避成功フラグ
	bool mEKeyWasDown;          // Eキー前回押下状態
	BossEnemySlime* mpBossSlime;// スポーンしたキングスライム
};
