#pragma once
// チュートリアルのテキストを管理するクラス
// ステップ形式でやるかは考え中

#include "StepData.h"
#include "StepLoader.h"
#include "DeathReason.h"
#include <DxLib.h>
#include <set>
#include <string>
#include <utility>
#include "PlayerActionState.h"

class  TutorialTextManager
{
private:
	// 定数
	static constexpr float CharSpeed = 0.02f; // 一文字表示にかける時間

	// 座標イベントの識別キー（軸, 座標値）
	// falseならX軸、trueならY軸
	using CoordKey = std::pair<bool, float>;

public:

	// 死亡シーンでテキストの位置を分けるため
	enum DrawPosition
	{
		TEXT_UP,
		TEXT_DOWN,
	};

	// 洞窟ステージにおける「穴に落ちる前」「落ちた後」を管理するフェーズ
	enum class CaveFallPhase
	{
		None,       // 洞窟ステージ以外（森など）
		BeforeFall, // オープニング：穴に落ちる前
		AfterFall,  // 本編：穴に落ちた後
	};

	void Initialize(
		const std::string& csvPath,
		const std::string& deathCsvPath
	);

	void Update(float dt, float playerX, float playerY);
	void Draw(DrawPosition DrawPoisition);
	void DebugDraw();

	// 文字を進める処理
	void UpdateString(const StepData* step);
	// 次のステップに進める
	void NextStepUpdate(bool mbCompleted);

	// ステップの変更
	void ChangeStep(int nextID);

	// プレイヤー復活時
	void OnPlayerRevive();

	// プレイヤー死亡時
	void OnPlayerDead(DeathReason::DeathType reason);

	// ボス撃破時
	void OnBossDefeated();

	// 現在のステップを返す
	const StepData* GetCurrentStep() const;

	// 全ての文字が表示されたかのアクセサ
	bool GetIsTextCompleted() { return mbIsTextCompleted; }

	// チュートリアルの状態を初期状態へリセットするため
	void Reset();

	void SetActionState(const PlayerActionState& state);

	// 洞窟落下フェーズのセッター・ゲッター
	void SetCaveFallPhase(CaveFallPhase phase) { mCaveFallPhase = phase; }
	CaveFallPhase GetCaveFallPhase() const { return mCaveFallPhase; }

	// 穴に落ちる前かどうか
	bool IsBeforeFall() const { return mCaveFallPhase == CaveFallPhase::BeforeFall; }
	// 穴に落ちた後かどうか
	bool IsAfterFall()  const { return mCaveFallPhase == CaveFallPhase::AfterFall; }



private:
	// トリガータイプが座標（X）によるものかを判定するため
	bool IsXTriggerType(const std::string& type) const;

	// トリガータイプが座標（Y）によるものかを判定するため
	bool IsYTriggerType(const std::string& type) const;

	// トリガータイプが座標（X または Y）によるものかを判定するため
	bool IsPositionTriggerType(const std::string& type) const;

	// トリガーの軸に対応するプレイヤー座標を取得するため
	float GetAxisPosition(const std::string& type, float playerX, float playerY) const;

	// トリガータイプが時間によるものかを判定するため
	bool IsTimeTriggerType(const std::string& type) const;

	// 現在のステップの進行条件を満たしているかを判定するため
	bool CheckCondition(const std::string& type, float targetValue, bool isCurrentlyTalking) const;

	// 条件分岐に応じて次に進むべきステップのIDを解決するため
	int ResolveNextStepID(int candidateID, float playerX, float playerY) const;

	// 死亡回数によって表示テキストを分ける
	void SelectDeathCountText();

	DrawPosition mDrawPoisition;

	// 洞窟における穴落下フェーズ（落ちる前 / 落ちた後）
	CaveFallPhase mCaveFallPhase = CaveFallPhase::None;

	// 通常チュートリアル用
	StepLoader loader;
	// 死亡時チュートリアル用
	StepLoader deathLoader;

	const StepData* mStep = nullptr;

	int mnCurrentID = 1; // 現在のステップID
	int mnPreviousID = 1; // 死亡する前の通常チュートリアルID
	bool mbIsDeathTutorial = false; // 現在、死亡時チュートリアルを表示しているか
	bool mbIsDeathTextFinished = false; // 死亡チュートリアルの表示が最後まで終わったか
	int mnDisplayByteCount = 0; // 画面に表示する文字列のバイト数


	float mfCurrentTriggerX = 0.0f; // 現在発生中のX座標グループを管理するため
	float mfCurrentTriggerY = 0.0f; // 現在発生中のY座標グループを管理するため
	bool  mbCurrentTriggerIsY = false; // 直近のトリガーがY軸かどうか
	std::set<int> mCompletedXStepIDs; // 既に表示開始した座標ステップのID履歴を保持するため


	float mfIdelTimer = 0.0f; // ステップが始まってからの経過時間
	float mfCharSpeed = CharSpeed; // 1文字を表示するのにかける時間
	float mfCharTimer = 0.0f; // 1文字表示するまでの時間
	bool mbIsTextCompleted = false; // 全ての文字が表示されたかのフラグ

	// Updateの保存メンバ
	float mfDeltaTime = 0.0f;
	float mfPlayerX = 0.0f;
	float mfPlayerY = 0.0f;

	// あとで別CPPに変更する
	// チュートリアル中のIF分岐判定に用いるプレイヤーの行動フラグ
	bool mbUsedMove = false;   // 移動を行ったか
	bool mbUsedBlink = false;  // ブリンクを行ったか
	bool mbUsedJump = false;   // ジャンプを行ったか
	bool mbUsedAttack = false; // 攻撃を行ったか

	// 山々山の処理
	bool mbWallBreak = false;  // 壁を壊したか
	bool mbHotSpring = false;  // 温泉に入ったか


	bool mbInterruptedLastX = false; // 直前の説明の途中で現在の座標に到達したか

	// 特定の座標（軸・値）のセリフが出た後に死んだかどうかを記録するセット
	std::set<CoordKey> mVisitedCoords;   // 1度でもセリフが表示された座標
	std::set<CoordKey> mDiedAfterCoords; // その座標のセリフが表示された後に1回以上死んだ座標

	std::string mDisplayText;
};