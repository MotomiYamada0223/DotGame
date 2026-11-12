#pragma once
#include "DeathReason.h"

class TutorialTextManager;

/// <summary>
/// 死亡時の天使カットイン演出を担当するクラス
/// 白フラッシュ → 八頭身天使＋死因テキスト（時止め）  白フラッシュ
/// 飛び散り・GAME OVER は Player 側で行う
/// </summary>
class DeathController
{
public:
	// 天使の表示モーション（死因ごとに選ばれる）
	enum class AngelMotion
	{
		Normal, // ノーマル（上半身のまばたき・口パク）
		Laugh,  // 爆笑
		Point,  // 指さし
		Hide    // 隠れ（あきれ）
	};

	enum class Phase
	{
		None,       // 演出なし
		PreFlash,   // 天使登場前の白フラッシュ
		CutIn,      // 天使カットイン＋死因テキスト（時止め）
		PostFlash,  // テキスト終了後の白フラッシュ
		Finished    // 演出終了（Playerが飛び散りへ移行する）
	};

	struct FrameContext
	{
		TutorialTextManager* text = nullptr; // 天使のセリフ
	};

	bool IsCutInActive() const { return mPhase == Phase::CutIn; }

	DeathController();
	~DeathController();

	void Start(DeathReason::DeathType type, AngelMotion motion);
	void Update(const FrameContext& ctx);
	void Draw(TutorialTextManager* text) const;
	void Reset();

	Phase GetPhase() const { return mPhase; }
	bool IsFinished() const { return mPhase == Phase::Finished; }

	// 天使・テキストを描画すべき段階か（PreFlash / CutIn / PostFlash）
	bool IsAngelCutInStage() const
	{
		return mPhase == Phase::PreFlash
			|| mPhase == Phase::CutIn
			|| mPhase == Phase::PostFlash;
	}

private:
	void LoadGraphs();
	void ReleaseGraphs();
	void UpdateAngelAnimation(bool textFinished);
	void DrawCutInScene(bool textFinished) const;
	void DrawAngel(bool textFinished) const;
	void DrawTextBox(bool textFinished) const;

	Phase mPhase;
	int mPreFlashTimer;
	int mPostFlashTimer;
	int mCutInTimer;
	int mPostTextTimer;
	DeathReason::DeathType mDeathType;
	AngelMotion mMotion;

	// 天使のアニメーション状態
	int mAngelAnimState;
	int mAngelTalkTimer;
	int mAngelBlinkTimer;
	bool mbAngelBlinking;

	// 天使・セリフ枠の画像
	int mAngelNormalHandles[3];
	int mAngelLaughHandles[2];
	int mAngelPointHandles[2];
	int mAngelHideHandles[2];
	int mAngelTextBoxHandle;
};