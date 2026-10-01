#pragma once
// チュートリアルのテキストを管理するクラス
// ステップ形式でやるかは考え中

#include "StepData.h"
#include "StepLoader.h"

class TutorialTextManager
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

	// プレイヤー死亡時
	void OnPlayerDead();

	// 現在のステップを返す
	const StepData* GetCurrentStep() const;

private:

	// 通常チュートリアル用
	StepLoader loader;
	// 死亡時チュートリアル用
	StepLoader deathLoader;



	int mnCurrentID = 1; // 現在のステップID
	int mnPreviousID = 1; // 死亡する前の通常チュートリアルID
	bool mbIsDeathTutorial = false; // 現在、死亡時チュートリアルを表示しているか

	int mnDisplayByteCount = 0; // 画面に表示する文字列のバイト数

	float mfIdelTimer = 0.0f; // ステップが始まってからの経過時間
	float mfCharSpeed = 0.05f; // 1文字を表示するのにかける時間
	float mfCharTimer = 0.0f; // 1文字表示するまでの時間
};