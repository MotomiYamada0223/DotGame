#include "TutorialTextManager.h"
#include "Master.h"
#include "GameConstants.h"


// ステップの取得処理を共通化
const StepData* TutorialTextManager::GetCurrentStep() const
{
	// 現在、死亡チュートリアル中かどうかで使用するCSVを切り替える
	if (mbIsDeathTutorial)
	{
		auto itr = deathLoader.steps.find(mnCurrentID);

		if (itr == deathLoader.steps.end())
		{
			return nullptr;
		}

		return &itr->second;
	}

	// 通常チュートリアル
	auto itr = loader.steps.find(mnCurrentID);

	if (itr == loader.steps.end())
	{
		return nullptr;
	}

	return &itr->second;
}


// Shift-JISの文字コードから文字のバイト数を判別する処理
// 1文字が2バイトで構成されているので
// 2バイト文字だと分かったら一気に進めるため
static int GetCharByteCount(const std::string& text, int index)
{
	unsigned char c = text[index];

	// Shift-JISの2バイト文字の範囲内か判定する
	if ((c >= 0x81 && c <= 0x9F) ||
		(c >= 0xE0 && c <= 0xEF))
	{
		return 2;
	}

	// 1バイトの英数字や記号として扱う
	return 1;
}


void TutorialTextManager::Initialize(
	const std::string& csvPath,
	const std::string& deathCsvPath)
{
	// メンバ変数の初期化
	mnCurrentID = 1;
	mnPreviousID = 1;

	mbIsDeathTutorial = false;
	mbIsDeathTextFinished = false;

	mfIdelTimer = 0.0f;
	mfCharSpeed = 0.05f;
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;

	// 通常チュートリアルCSVの読み込み
	loader.LoadCSV(csvPath);

	// 死亡時チュートリアルCSVの読み込み
	deathLoader.LoadCSV(deathCsvPath);
}


// プレイヤーが死亡したときの処理
void TutorialTextManager::OnPlayerDead()
{
	// 通常チュートリアル中なら、現在のIDを保存する
	if (!mbIsDeathTutorial)
	{
		mnPreviousID = mnCurrentID;
	}

	// 死亡チュートリアルへ切り替える
	mbIsDeathTutorial = true;

	// 死亡テキストはまだ終了していない
	mbIsDeathTextFinished = false;

	// 死亡CSVの最初のIDから開始
	mnCurrentID = 1;

	// 表示状態をリセット
	mfIdelTimer = 0.0f;
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;
}


// プレイヤーが復活したときの処理
void TutorialTextManager::OnPlayerRevive()
{
	// 通常チュートリアルへ戻る
	mbIsDeathTutorial = false;
	mbIsDeathTextFinished = false;

	// 死亡前に保存しておいたIDへ戻る
	mnCurrentID = mnPreviousID;

	// 表示状態をリセット
	mfIdelTimer = 0.0f;
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;
}


// 更新処理
void TutorialTextManager::Update(float dt)
{
	if (mbIsDeathTextFinished && mbIsDeathTutorial)
	{
		return;
	}

	// 現在のステップを取得する
	const StepData* step = GetCurrentStep();

	// ステップが存在しない場合は何もしない
	if (step == nullptr)
	{
		return;
	}

	// 一定速度で文字を進めるため、経過時間を加算する
	mfCharTimer += dt;

	// 指定の時間が経過したら1文字分のバイト数を進める
	if (mfCharTimer >= mfCharSpeed)
	{
		mfCharTimer -= mfCharSpeed;

		// すべての文字が表示しきっていない場合のみ処理する
		if (mnDisplayByteCount < step->text.size())
		{
			// マルチバイト文字を考慮して次の文字のバイト数を取得する
			int byteCount = GetCharByteCount(
				step->text,
				mnDisplayByteCount
			);

			mnDisplayByteCount += byteCount;
		}
	}

	// すべての文字が表示された後、
	// 次のステップへ移行するまでの余韻時間を計測する
	if (mnDisplayByteCount >= step->text.size())
	{
		mfIdelTimer += dt;

		// 待ち時間が完了したら次のステップへ切り替える
		if (mfIdelTimer >= step->completeValue)
		{
			ChangeStep(step->nextID);
			return;
		}
	}
}


// ステップを変更する/次のステップに進める処理
void TutorialTextManager::ChangeStep(int nextID)
{
	// 死亡チュートリアルが終了した場合
	if (mbIsDeathTutorial && nextID == 0)
	{
		mbIsDeathTextFinished = true;
		return;
	}


	// 現在使用しているCSVから次のステップを探す
	if (mbIsDeathTutorial)
	{
		auto itr = deathLoader.steps.find(nextID);

		if (itr == deathLoader.steps.end())
		{
			return;
		}
	}
	else
	{
		auto itr = loader.steps.find(nextID);

		if (itr == loader.steps.end())
		{
			return;
		}
	}


	// 存在していたら次のステップへ移行して時間をリセット
	mnCurrentID = nextID;
	mfIdelTimer = 0.0f;

	// 文字送りもリセット
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;
}


// 画面に表示するテキストの描画
void TutorialTextManager::Draw()
{
	// 現在のステップを探す
	unsigned int color = ColorOption::White;
	const StepData* step = GetCurrentStep();

	// ステップが存在しない場合は何もしない
	if (step == nullptr)
	{
		return;
	}

	// 通常のテキストは表示時間を越したら表示しない
	// ただし、死亡テキスト終了後は最後のテキストを表示し続ける
	if (!(mbIsDeathTutorial && mbIsDeathTextFinished) &&
		mfIdelTimer >= step->completeValue)
	{
		return;
	}


	// 現在表示すべきバイト数分だけ文字列を切り出す処理
	// 先頭の文字から出すべきバイト数まで。
	std::string displayText =
		step->text.substr(0, mnDisplayByteCount);

	// 切り出したテキストを指定位置に描画する
	Master::mpGameManager->GetFontManager()->DrawDotString(
		TutorialTextControll::TextX,
		TutorialTextControll::TextY,
		TutorialTextControll::TextSize,
		color,
		"%s",
		displayText.c_str());
}


// デバッグ描画
void TutorialTextManager::DebugDraw()
{
	const StepData* step = GetCurrentStep();

	// ステップが存在しない場合は何もしない
	if (step == nullptr)
	{
		return;
	}

	// 現在表示すべきバイト数分だけ文字列を切り出す処理
	std::string displayText =
		step->text.substr(0, mnDisplayByteCount);

	// デバッグ用の表示
	DrawFormatString(
		0,
		0,
		ColorOption::White,
		"ID: %d  表示時間: %f / %f",
		mnCurrentID,
		mfIdelTimer,
		step->completeValue
	);
}