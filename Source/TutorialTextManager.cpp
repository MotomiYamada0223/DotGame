#include "TutorialTextManager.h"
#include "Master.h"

// Shift-JISの文字コードから文字のバイト数を判別する処理
//1文字が2バイトで構成されているので2バイト文字だと分かったら一気に進めるため
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


void TutorialTextManager::Initialize(const std::string& csvPath)
{
	// メンバ変数の初期化
	mnCurrentID = 1;
	mfIdelTimer = 0.0f;
	mfCharSpeed = 0.05f;
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;

	// CSVの読み込み
	loader.LoadCSV(csvPath);
}

void TutorialTextManager::Update(float dt)
{
	// 現在のステップを探す
	auto itr = loader.steps.find(mnCurrentID);
	if (itr == loader.steps.end()) { return; }

	StepData& step = itr->second;


	// 一定速度で文字を進めるため、経過時間を加算する
	mfCharTimer += dt;
	// 指定の時間が経過したら1文字分のバイト数を進める
	if (mfCharTimer >= mfCharSpeed)
	{
		mfCharTimer -= mfCharSpeed;

		// すべての文字が表示しきっていない場合のみ処理する
		if (mnDisplayByteCount < step.text.size())
		{
			// マルチバイト文字を考慮して次の文字のバイト数を取得する
			int byteCount = GetCharByteCount(
				step.text,
				mnDisplayByteCount
			);
			mnDisplayByteCount += byteCount;
		}
	}

	// すべての文字が表示された後、次のステップへ移行するまでの余韻時間を計測する
	if (mnDisplayByteCount >= step.text.size())
	{
		mfIdelTimer += dt;

		// 待ち時間が完了したら次のステップへ切り替える
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

	// 文字送りもリセット
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;
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


	// 現在表示すべきバイト数分だけ文字列を切り出す処理
	// 先頭の文字から出すべきバイト数まで。mnDisplayByteCountが伸びていくので出る
	// substr...文字列の中から指定した一部分を切り出して新しい文字列を作る
	std::string displayText =
		step.text.substr(0, mnDisplayByteCount);
	// 切り出したテキストを指定位置に描画する
	DrawFormatStringToHandle(
		50,
		50,
		color,
		Master::mpFontManager->GetDotFont(),
		"%s",
		displayText.c_str()
	);


	// デバッグ用の表示
	DrawFormatString(0, 0, color, "ID: %d  表示時間: %f / %f", mnCurrentID, mfIdelTimer, step.completeValue);
}


