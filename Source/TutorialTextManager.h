#pragma once
// チュートリアルのテキストを管理するクラス
// ステップ形式でやるかは考え中

#include "StepData.h"
#include "StepLoader.h"
#include "DeathReason.h"
#include <DxLib.h>

class  TutorialTextManager
{
public:

	void Initialize(
		const std::string& csvPath,
		const std::string& deathCsvPath
	);

	void Update(float dt);
	void Draw();
	void DebugDraw();
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


private:

	// 通常チュートリアル用
	StepLoader loader;
	// 死亡時チュートリアル用
	StepLoader deathLoader;

	// 定数
	static constexpr float CharSpeed = 0.1f; // 一文字表示にかける時間


	int mnCurrentID = 1; // 現在のステップID
	int mnPreviousID = 1; // 死亡する前の通常チュートリアルID
	bool mbIsDeathTutorial = false; // 現在、死亡時チュートリアルを表示しているか
	bool mbIsDeathTextFinished = false; // 死亡チュートリアルの表示が最後まで終わったか

	int mnDisplayByteCount = 0; // 画面に表示する文字列のバイト数

	float mfIdelTimer = 0.0f; // ステップが始まってからの経過時間
	float mfCharSpeed = CharSpeed; // 1文字を表示するのにかける時間
	float mfCharTimer = 0.0f; // 1文字表示するまでの時間

	bool mbIsTextCompleted = false; // 全ての文字が表示されたかのフラグ
};