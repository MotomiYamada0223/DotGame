#pragma once
// チュートリアルのテキストを管理するクラス
// ステップ形式でやるかは考え中

#include "StepData.h"
#include "StepLoader.h"
#include "DeathReason.h"
#include <DxLib.h>
#include <set>
#include "PlayerActionState.h"

class  TutorialTextManager
{
private:
	// 定数
	static constexpr float CharSpeed = 0.1f; // 一文字表示にかける時間


public:

	void Initialize(
		const std::string& csvPath,
		const std::string& deathCsvPath
	);

	void Update(float dt, float playerX);
	void Draw();
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

	// 現在のステップを返す
	const StepData* GetCurrentStep() const;

	// 全ての文字が表示されたかのアクセサ
	bool GetIsTextCompleted() { return mbIsTextCompleted; }

	// チュートリアルの状態を初期状態へリセットするため
	void Reset();

	void SetActionState(const PlayerActionState& state);


private:
	// トリガータイプが座標（X）によるものかを判定するため
	bool IsXTriggerType(const std::string& type) const;

	// トリガータイプが時間によるものかを判定するため
	bool IsTimeTriggerType(const std::string& type) const;

	// 現在のステップの進行条件を満たしているかを判定するため
	bool CheckCondition(const std::string& type) const;


	// 条件分岐に応じて次に進むべきステップのIDを解決するため
	int ResolveNextStepID(int candidateID, float playerX) const;



	// 通常チュートリアル用
	StepLoader loader;
	// 死亡時チュートリアル用
	StepLoader deathLoader;

	const StepData* mStep;

	int mnCurrentID = 1; // 現在のステップID
	int mnPreviousID = 1; // 死亡する前の通常チュートリアルID
	bool mbIsDeathTutorial = false; // 現在、死亡時チュートリアルを表示しているか
	bool mbIsDeathTextFinished = false; // 死亡チュートリアルの表示が最後まで終わったか
	int mnDisplayByteCount = 0; // 画面に表示する文字列のバイト数


	float mfCurrentTriggerX; // 現在発生中のX座標グループを管理するため
	std::set<int> mCompletedXStepIDs; // 既に表示開始したX座標ステップのID履歴を保持するため


	float mfIdelTimer = 0.0f; // ステップが始まってからの経過時間
	float mfCharSpeed = CharSpeed; // 1文字を表示するのにかける時間
	float mfCharTimer = 0.0f; // 1文字表示するまでの時間
	bool mbIsTextCompleted = false; // 全ての文字が表示されたかのフラグ

	// Updateの保存メンバ
	float mfDeltaTime = 0.0f;
	float mfPlayerX = 0.0f;

	// あとで別CPPに変更する
	// チュートリアル中のIF分岐判定に用いるプレイヤーの行動フラグ
	bool mbUsedMove = false;   // 移動を行ったか
	bool mbUsedBlink = false;  // ブリンクを行ったか
	bool mbUsedJump = false;   // ジャンプを行ったか
	bool mbUsedAttack = false; // 攻撃を行ったか
};