#include "TutorialTextManager.h"
#include "Master.h"
#include "GameConstants.h"
#include "DeathReason.h"
#include "ColorTagParser.h"

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


// 文字列中のタグ（<red>や</>など）の長さを取得するため（タグでなければ0を返す）
static int GetTagLength(const std::string& text, int index)
{
	if (index >= static_cast<int>(text.size()) || text[index] != '<') return 0;
	size_t closePos = text.find('>', index);
	if (closePos != std::string::npos && closePos - index <= 12)
	{
		return static_cast<int>(closePos - index + 1);
	}
	return 0;
}


// 指定されたトリガータイプがX座標によるものかを判定するため
bool TutorialTextManager::IsXTriggerType(const std::string& type) const
{
	return (type == "X" || type.rfind("X_", 0) == 0);
}

// 指定されたトリガータイプが時間によるものかを判定するため
bool TutorialTextManager::IsTimeTriggerType(const std::string& type) const
{
	return (type == "Time" || type.rfind("Time_", 0) == 0);
}



// プレイヤーの行動フラグなどに基づいてIF分岐条件を満たしているかを判定するため
bool TutorialTextManager::CheckCondition(const std::string& type) const
{
	if (type == "X" || type == "Time") return true;

	std::string cond = type;
	if (cond.rfind("X_", 0) == 0) cond = cond.substr(2);
	else if (cond.rfind("Time_", 0) == 0) cond = cond.substr(5);

	if (cond == "Blink")    return mbUsedBlink;
	if (cond == "NoBlink")  return !mbUsedBlink;
	if (cond == "Jump")     return mbUsedJump;
	if (cond == "NoJump")   return !mbUsedJump;
	if (cond == "Attack")   return mbUsedAttack;
	if (cond == "NoAttack") return !mbUsedAttack;
	if (cond == "Move")     return mbUsedMove;
	if (cond == "NoMove")   return !mbUsedMove;


	return true;
}

void TutorialTextManager::SetActionState(const PlayerActionState& state)
{
	mbUsedMove |= state.move;
	mbUsedBlink |= state.blink;
	mbUsedJump |= state.jump;
	mbUsedAttack |= state.attack;
}


// 次のステップID候補から条件を満たす分岐先を自動で解決、選択するため
int TutorialTextManager::ResolveNextStepID(int candidateID, float playerX) const
{
	for (int offset = 0; offset < 5; ++offset)
	{
		int checkID = candidateID + offset;
		auto itr = loader.steps.find(checkID);
		if (itr == loader.steps.end()) break;

		const StepData& s = itr->second;
		if (IsTimeTriggerType(s.completeType))
		{
			if (CheckCondition(s.completeType))
			{
				return checkID;
			}
		}
		else if (IsXTriggerType(s.completeType))
		{
			// まだ到達していない未来のX座標ステップへ誤って飛ばないようにするため
			if (s.completeValue > playerX)
			{
				return 0;
			}
			if (CheckCondition(s.completeType))
			{
				return checkID;
			}
		}
	}
	return 0;
}


void TutorialTextManager::Initialize(
	const std::string& csvPath,
	const std::string& deathCsvPath)
{
	// メンバ変数の初期化
	mnCurrentID = 0;
	mnPreviousID = 0;
	mCompletedXStepIDs.clear();

	mbIsDeathTutorial = false;
	mbIsDeathTextFinished = false;

	mfIdelTimer = 0.0f;
	mfCharSpeed = CharSpeed;
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;

	mbUsedMove = false;
	mbUsedBlink = false;
	mbUsedJump = false;
	mbUsedAttack = false;

	// 通常チュートリアルCSVの読み込み
	loader.LoadCSV(csvPath);

	// 死亡時チュートリアルCSVの読み込み
	deathLoader.LoadCSV(deathCsvPath);
}


// プレイヤーが死亡したときの処理
// 判別するために引数を取得
void TutorialTextManager::OnPlayerDead(DeathReason::DeathType type)
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
	DeathReason deathReason;
	mnCurrentID = deathReason.GetDeathStartID(type);

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
void TutorialTextManager::Update(float dt, float playerX)
{
	if (mbIsDeathTextFinished && mbIsDeathTutorial)
	{
		return;
	}

	mfDeltaTime = dt;
	mfPlayerX = playerX;

	// プレイヤーのX座標に応じたステップの発生条件をチェックして切り替えるため
	int bestID = 0;
	float bestX = mfCurrentTriggerX;

	for (const auto& pair : loader.steps)
	{
		const StepData& s = pair.second;
		bool isAvailable = (IsXTriggerType(s.completeType) && CheckCondition(s.completeType) &&
			mCompletedXStepIDs.find(s.id) == mCompletedXStepIDs.end());

		if (isAvailable)
		{
			if (mfPlayerX >= s.completeValue && s.completeValue > mfCurrentTriggerX)
			{
				if (s.completeValue > bestX || (s.completeValue == bestX && (bestID == 0 || s.id < bestID)))
				{
					bestX = s.completeValue;
					bestID = s.id;
				}
			}
		}
	}

	if (bestID != 0)
	{
		mfCurrentTriggerX = bestX;

		// 同じX座標にある別の分岐行も完了扱いに登録して二重発生条件を防ぐため
		for (const auto& pair : loader.steps)
		{
			const StepData& s = pair.second;
			if (IsXTriggerType(s.completeType) && s.completeValue == bestX)
			{
				if (!CheckCondition(s.completeType))
				{
					mCompletedXStepIDs.insert(s.id);
				}
			}
		}

		mCompletedXStepIDs.insert(bestID);
		ChangeStep(bestID);
	}

	// 現在のステップを取得する
	mStep = GetCurrentStep();

	// ステップが存在しない場合は何もしない
	if (mStep == nullptr)
	{
		return;
	}
	// 文字の更新
	UpdateString(mStep);

	mbIsTextCompleted = (mnDisplayByteCount >= static_cast<int>(mStep->text.size()));
	NextStepUpdate(mbIsTextCompleted);
}



void TutorialTextManager::NextStepUpdate(bool mbCompleted)
{
	// すべての文字が表示された後、
	// 次のステップへ移行するまでの余韻時間を計測する
	if (mbCompleted)
	{
		mfIdelTimer += mfDeltaTime;

		float waitDuration = mStep->completeValue;
		if (IsTimeTriggerType(mStep->completeType) && mStep->completeValue > 0.0f)
		{
			waitDuration = mStep->completeValue;
		}
		else if (mStep->nextID != 0)
		{
			waitDuration = 3.0f; // デフォルトの余韻時間
		}

		// 待ち時間が完了したら次のステップへ切り替える
		if (mfIdelTimer >= waitDuration)
		{
			// 次のステップIDが指定されている場合、条件を満たす分岐先へ進むため
			if (mStep->nextID != 0)
			{
				int resolvedID = ResolveNextStepID(mStep->nextID, mfPlayerX);
				if (resolvedID != 0)
				{
					mCompletedXStepIDs.insert(resolvedID);
					ChangeStep(resolvedID);
					return;
				}
			}

			// 同じX座標に設定された条件を満たす未表示の別行があれば自動で次へ進めるため
			int sameXNextID = 0;
			for (const auto& pair : loader.steps)
			{
				const StepData& s = pair.second;
				if (IsXTriggerType(s.completeType) && CheckCondition(s.completeType) &&
					s.completeValue == mfCurrentTriggerX && mCompletedXStepIDs.find(s.id) == mCompletedXStepIDs.end())
				{
					if (sameXNextID == 0 || s.id < sameXNextID)
					{
						sameXNextID = s.id;
					}
				}
			}

			if (sameXNextID != 0)
			{
				mCompletedXStepIDs.insert(sameXNextID);
				ChangeStep(sameXNextID);
				return;
			}

			// 次のステップへ進むか判定
			if (mStep->nextID != 0)
			{
				bool waitForFutureX = false;

				// nextID以降に、まだ到達していないXトリガーがあるか確認
				for (int offset = 0; offset < 5; ++offset)
				{
					int checkID = mStep->nextID + offset;

					auto itr = loader.steps.find(checkID);
					if (itr == loader.steps.end()) break;

					const StepData& nextCandidate = itr->second;

					if (IsXTriggerType(nextCandidate.completeType) &&
						nextCandidate.completeValue > mfPlayerX)
					{
						waitForFutureX = true;
						break;
					}
				}

				// 未来のX座標を待つ場合は、現在のテキストを維持する
				if (waitForFutureX)
				{
					return;
				}

				// 次のステップが存在する場合のみ進む
				auto itr = mbIsDeathTutorial
					? deathLoader.steps.find(mStep->nextID)
					: loader.steps.find(mStep->nextID);

				const auto& steps = mbIsDeathTutorial
					? deathLoader.steps
					: loader.steps;

				if (itr != steps.end())
				{
					ChangeStep(mStep->nextID);
					return;
				}
			}
			// 次のステップがない場合は、現在のテキストを維持する
			return;
		}
	}
}

void TutorialTextManager::UpdateString(const StepData* step)
{
	// 一定速度で文字を進めるため、経過時間を加算する
	mfCharTimer += mfDeltaTime;

	// 指定の時間が経過したら1文字分のバイト数を進める
	while (mfCharTimer >= mfCharSpeed)
	{
		mfCharTimer -= mfCharSpeed;

		// すべての文字が表示しきっていない場合のみ処理する
		if (mnDisplayByteCount < static_cast<int>(step->text.size()))
		{
			// 色タグ部分に差し掛かった場合は一瞬でスキップして進めるため
			while (mnDisplayByteCount < static_cast<int>(step->text.size()))
			{
				int tagLen = GetTagLength(step->text, mnDisplayByteCount);
				if (tagLen > 0)
				{
					mnDisplayByteCount += tagLen;
				}
				else
				{
					break;
				}
			}

			if (mnDisplayByteCount < static_cast<int>(step->text.size()))
			{
				// マルチバイト文字を考慮して次の文字のバイト数を取得する
				int byteCount = GetCharByteCount(
					step->text,
					mnDisplayByteCount
				);
				mnDisplayByteCount += byteCount;
			}

			// 文字を進めた後も次の位置にタグがあればさらにスキップする
			while (mnDisplayByteCount < static_cast<int>(step->text.size()))
			{
				int tagLen = GetTagLength(step->text, mnDisplayByteCount);
				if (tagLen > 0)
				{
					mnDisplayByteCount += tagLen;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// 最後まで表示しきったらタイマーリセット
			mfCharTimer = 0.0f;
			break;
		}
	}
}


// チュートリアルの進行状態やプレイヤーの行動フラグを初期状態に戻すため
void TutorialTextManager::Reset()
{
	mnCurrentID = 0;
	mfCurrentTriggerX = 0;
	mCompletedXStepIDs.clear();
	mfIdelTimer = 0.0f;
	mfCharTimer = 0.0f;
	mnDisplayByteCount = 0;
	mbUsedMove = false;
	mbUsedBlink = false;
	mbUsedJump = false;
	mbUsedAttack = false;
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



void TutorialTextManager::Draw()
{
	const StepData* step = GetCurrentStep();

	if (step == nullptr) return;
	if (step->text == "none" ||
		step->text == "0" ||
		step->text.empty())
	{
		return;
	}

	// 
	/*if (!(mbIsDeathTutorial && mbIsDeathTextFinished) &&
		mfIdelTimer >= step->completeValue)
	{
		return;
	}*/

	std::string displayText =
		step->text.substr(0, mnDisplayByteCount);

	// フォント設定
	int fontSize = TutorialTextControll::TextSize;
	int fontHandle =
		Master::mpGameManager->GetFontManager()
		->GetDotFontHandle(fontSize);

	if (fontHandle == -1) return;

	int baseX = TutorialTextControll::TextX;
	int curX = baseX;
	int curY = TutorialTextControll::TextY;

	unsigned int color = GetColor(255, 255, 255);

	std::string chunk;
	int index = 0;
	int textLength = static_cast<int>(displayText.size());

	// ためた文字列を現在の位置・色で描画する
	auto flushChunk = [&]()
		{
			if (chunk.empty()) return;

			Master::mpGameManager->GetFontManager()
				->DrawDotString(
					curX,
					curY,
					fontSize,
					color,
					chunk.c_str()
				);

			// 描画した文字列の幅だけ次の描画位置を右へ移動
			curX += GetDrawStringWidthToHandle(
				chunk.c_str(),
				static_cast<int>(chunk.size()),
				fontHandle
			);

			chunk.clear();
		};

	while (index < textLength)
	{
		// 改行処理
		if (displayText[index] == '\n')
		{
			flushChunk();
			curX = baseX;
			curY += fontSize + 6;
			index++;
			continue;
		}

		// TutorialTextManager.cpp での使用例
		int tagLen = ColorTagParser::GetTagLength(displayText, index);
		if (tagLen > 0)
		{
			// 色が変わる前に、ここまでの文字列を現在の色で確定させて描画するため
			flushChunk();

			std::string tagContent = displayText.substr(index + 1, tagLen - 2);
			unsigned int newColor = color;
			if (ColorTagParser::ParseColorTag(tagContent, newColor))
			{
				color = newColor;
			}
			index += tagLen;
		}

		// Shift-JISの文字を途中で分割しない
		int charBytes = GetCharByteCount(displayText, index);

		chunk.append(displayText, index, charBytes);
		index += charBytes;
	}

	// 最後に残った文字列を描画
	flushChunk();
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