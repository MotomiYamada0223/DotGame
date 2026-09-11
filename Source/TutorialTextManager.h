#pragma once

// チュートリアルのテキストを管理するクラス
// ステップ形式でやるかは考え中

#include "StepData.h"
#include "StepLoader.h"
#include "DxLib.h"

class TutorialTextManager
{
public:

	void Initialize(const std::string& csvPath);
	void Update(float dt);
	void Draw();

	// ステップの変更
	void ChangeStep(int nextID);
	// 現在の文章を取得
	const std::string& GetCurrentText() const;

private:
	StepLoader loader;

	int mnCurrentID = 1; // 現在のステップのID
	int mnDisplayByteCount = 0; // 画面に表示する文字列のバイト数

	float mfIdelTimer = 0.0f; // ステップが始まってからの経過時間
	float mfCharSpeed = 0.05f; // 1文字を表示するのにかける時間
	float mfCharTimer = 0.0f; // 1文字表示するまでの時間
};