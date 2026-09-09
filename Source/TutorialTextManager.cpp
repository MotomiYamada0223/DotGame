#include "TutorialTextManager.h"
#include "Master.h"

void TutorialTextManager::Initialize(const std::string& csvPath)
{
	// メンバ変数の初期化
	mnCurrentID = 1;
	mfIdelTimer = 0.0f;

	// CSVの読み込み
	loader.LoadCSV(csvPath);
}

void TutorialTextManager::Update(float dt)
{
	// 現在のステップを探す
	auto itr = loader.steps.find(mnCurrentID);
	if (itr == loader.steps.end()) { return; }

	StepData& step = itr->second;

	// CompleteTypeのタイプによって次のステップに進む方法を変更するため
	
	// Timeの場合
	// 指定された時間が景かしたら次のステップへ進める処理
	if (step.completeType == "Time")
	{
		mfIdelTimer += dt;
		if (mfIdelTimer >= step.completeValue)
		{
			ChangeStep(step.nextID);
			return;
		}
	}
}

// ステップを変更する/次のステップに進める処理
void TutorialTextManager::ChangeStep(int nextID)
{
	// 移動先のステップが存在するか
	auto itr = loader.steps.find(nextID);
	if (itr == loader.steps.end()) { return; }

	// 存在していたら次のステップへ移行して時間をリセット
	mnCurrentID = nextID;
	mfIdelTimer = 0.0f;
}

// 今の文章を取得する処理
const std::string& TutorialTextManager::GetCurrentText() const
{
	// 現在のステップを探す
	auto itr = loader.steps.find(mnCurrentID);
	if (itr == loader.steps.end())
	{
		static std::string emptyText;
		return emptyText;
	}
	return itr->second.text;
}

// 画面に表示するテキストの描画
void TutorialTextManager::Draw()
{
	// 現在のステップを探す
	auto itr = loader.steps.find(mnCurrentID);
	if (itr == loader.steps.end()) { return; }
	
	unsigned int color = GetColor(255, 255, 255);
	const StepData& step = itr->second;
	// 表示時間を越していたら表示しない
	if (mfIdelTimer >= step.completeValue) { return; }

	
	DrawFormatStringToHandle(50, 50, color, Master::mpFontManager->GetDotFont(),
		"%s", step.text.c_str());

	// デバッグ用の表示
	DrawFormatString(0, 0, color, "ID: %d  表示時間: %f / %f", mnCurrentID, mfIdelTimer, step.completeValue);
}


