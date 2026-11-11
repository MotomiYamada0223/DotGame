#include "WinResultScene.h"
#include "InputManager.h"
#include "Master.h"
#include "GameConstants.h"
#include "Utility.h"

WinResultScene::WinResultScene()
	:Scene()
	, mSaint(VGet(Utility::SCREEN_WIDTH / 2.0f, 150.0f, 0.0f)) // 天使の作成
{

}

WinResultScene::~WinResultScene()
{

}

void WinResultScene::Initialize()
{
	// 天使のセッターとテキストのゲッターの紐づけをする
	mSaint.SetIsTextCompletedChecker([this]()
	{
		if (mCurrentLine < 0 || mCurrentLine >= static_cast<int>(mMessages.size()))
		{
			return true;
		}
		return mDisplayByteCount >= static_cast<int>(mMessages[mCurrentLine].size());
	});



	// UI枠の画像をロード
	mPortraitBoxHandle = LoadGraph(UIGraphPath::SaintFrame.c_str());
	mTextBoxHandle = LoadGraph(UIGraphPath::TextFrame.c_str());

	// 0: 目閉じ, 1: 目開き・口閉じ, 2: 目開き・口開き
	// 表情差分画像を配列に割り当ててロード
	mFullBodyHandles[0] = LoadGraph("Resource/Saint/Idel/angel_fullbody_closed.png");
	mFullBodyHandles[1] = LoadGraph("Resource/Saint/Idel/angel_fullbody_open.png");
	mFullBodyHandles[2] = LoadGraph("Resource/Saint/Idel/angel_fullbody_talk.png");

	

	// 画面外から中央へキャラを進入させる初期位置の設定
	mAngelX = static_cast<float>(ScreenSize::ScrrenWidth) + 300.0f;
	mAngelY = 635.0f;
	mHasArrived = false;
	mArriveWaitTimer = 0;

	// アニメーションおよび瞬き状態の初期化
	mCurrentState = 1;
	mTalkTimer = 0;
	mBlinkTimer = 0;
	mIsBlinking = false;
	mBlinkInterval = 180 + GetRand(60);

	// 表示メッセージリストの作成
	mMessages.clear();
	mMessages.push_back("ステージクリアおめでとう！\nよく頑張ったわね！");
	char deathMessage[256];
	sprintf_s(
		deathMessage,
		"あなたは今回のステージで%d回死亡したわ",
		Master::gDeathCounts[Master::gCurrentStageIndex]
	);
	mMessages.push_back(deathMessage);
	mMessages.push_back("次のステージも頑張ってね！応援しているわ！");

	// テキスト送り・進行制御タイマーの初期化
	mCurrentLine = 0;
	mDisplayByteCount = 0;
	mCharTimer = 0.0f;
	mCharSpeed = 0.05f;
	mWaitTimer = 0.0f;
	mIsTransitioning = false;
	mTransitionTimer = 0;
}

void WinResultScene::Update()
{
	mSaint.Update(); // 天使の更新

	// シーン遷移中のフェード処理
	if (mIsTransitioning)
	{
		mTransitionTimer++;
		if (mTransitionTimer >= 60)
		{
			// 規定フレーム経過後にステージ選択画面へ移行
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_STAGESELECT);
		}
		return;
	}

	// キャラクターの定期的な瞬き更新処理
	if (mIsBlinking)
	{
		mBlinkTimer++;
		if (mBlinkTimer >= 6)
		{
			// 目を閉じる時間が終了したら目開き状態へ戻す
			mIsBlinking = false;
			mBlinkTimer = 0;
			mBlinkInterval = 180 + GetRand(60);
		}
		mCurrentState = 0;
	}
	else
	{
		mBlinkTimer++;
		if (mBlinkTimer >= mBlinkInterval)
		{
			// 次の瞬きタイミングに到達したら目を閉じる
			mIsBlinking = true;
			mBlinkTimer = 0;
		}
	}

	// 画面右外から中央へのキャラクター移動フェーズ
	if (!mHasArrived)
	{
		float targetX = static_cast<float>(ScreenSize::CenterX);
		if (mAngelX > targetX)
		{
			float speed = 14.0f;
			mAngelX -= speed;
			if (mAngelX <= targetX)
			{
				mAngelX = targetX;
			}
		}
		else
		{
			// 到着後の間を設けるためのカウント
			mArriveWaitTimer++;
			if (mArriveWaitTimer >= 25)
			{
				mHasArrived = true;
			}
		}

		if (!mIsBlinking)
		{
			mCurrentState = 1;
		}
		Scene::Update();
		return;
	}

	// 会話進行フェーズ（中央移動完了後に実行）
	const std::string& currentText = mMessages[mCurrentLine];
	bool isTyping = (mDisplayByteCount < static_cast<int>(currentText.size()));

	// 口パクアニメーションの制御
	if (!mIsBlinking)
	{
		if (isTyping)
		{
			mTalkTimer++;
			if (mTalkTimer >= 12)
			{
				// 文字表示中は口を開閉させる
				mTalkTimer = 0;
				mCurrentState = (mCurrentState == 1) ? 2 : 1;
			}
		}
		else
		{
			// 文字表示が完了したら口を閉じる
			mCurrentState = 1;
		}
	}

	float dt = 1.0f / 60.0f;
	if (isTyping)
	{
		mCharTimer += dt;
		if (mCharTimer >= mCharSpeed)
		{
			mCharTimer -= mCharSpeed;

			// 文字コードの先頭バイトを判定し、全角・マルチバイト文字の途中切れを防ぐ文字送り処理
			if (mDisplayByteCount < static_cast<int>(currentText.size()))
			{
				unsigned char leadChar = static_cast<unsigned char>(currentText[mDisplayByteCount]);
				// Shift-JIS 2バイト文字判定
				if ((leadChar >= 0x81 && leadChar <= 0x9F) || (leadChar >= 0xE0 && leadChar <= 0xFC))
				{
					mDisplayByteCount += 2;
				}
				// UTF-8 3バイト文字判定
				else if ((leadChar & 0xF0) == 0xE0)
				{
					mDisplayByteCount += 3;
				}
				// UTF-8 2バイト文字判定
				else if ((leadChar & 0xE0) == 0xC0)
				{
					mDisplayByteCount += 2;
				}
				// 半角英数等 1バイト文字
				else
				{
					mDisplayByteCount += 1;
				}
			}

			if (mDisplayByteCount > static_cast<int>(currentText.size()))
			{
				mDisplayByteCount = static_cast<int>(currentText.size());
			}
		}

		// 決定キー入力時に一括で全文を表示するスキップ機能
		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) || InputManager::CheckDownKey(KEY_INPUT_SPACE))
		{
			mDisplayByteCount = static_cast<int>(currentText.size());
		}
	}
	else
	{
		// 文字全文表示後のキー入力・時間経過によるメッセージ送り処理
		mWaitTimer += dt;
		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) || InputManager::CheckDownKey(KEY_INPUT_SPACE) || mWaitTimer >= 200.5f)
		{
			mCurrentLine++;
			if (mCurrentLine >= static_cast<int>(mMessages.size()))
			{
				// 全メッセージ終了後は暗転フェードアウトを開始
				mIsTransitioning = true;
				mTransitionTimer = 0;
				mCurrentLine = static_cast<int>(mMessages.size()) - 1;
			}
			else
			{
				// 次のメッセージへ進めるためのカウンター初期化
				mDisplayByteCount = 0;
				mCharTimer = 0.0f;
				mWaitTimer = 0.0f;
			}
		}
	}

	Scene::Update();
}


void WinResultScene::Draw()
{
	// 背景の全画面黒塗り
	DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);

	// 八頭身天使キャラクターの描画
	if (mCurrentState >= 0 && mCurrentState < 3 && mFullBodyHandles[mCurrentState] != -1)
	{
		DrawRotaGraph(static_cast<int>(mAngelX), static_cast<int>(mAngelY), 0.72, 0.0, mFullBodyHandles[mCurrentState], TRUE);
	}

	
	// 中央到着後のメッセージ・UI枠描画
	if (mHasArrived)
	{
		// 天使のフレームを描画
		DrawGraph(
			UIGraphController::SaintFramePosX,
			UIGraphController::FramePosY,
			mPortraitBoxHandle,
			true
		);

		// テキストのフレームを描画
		DrawGraph(
			UIGraphController::TextFramePosX,
			UIGraphController::FramePosY,
			mTextBoxHandle,
			true
		);
		mSaint.Draw();

		// 現在表示進行中の範囲までメッセージを描画
		if (mCurrentLine >= 0 && mCurrentLine < static_cast<int>(mMessages.size()))
		{
			std::string displayText = mMessages[mCurrentLine].substr(0, mDisplayByteCount);

			Master::mpGameManager->GetFontManager()->DrawDotString(
				TutorialTextControll::TextX,
				TutorialTextControll::TextY,
				TutorialTextControll::TextSize,
				ColorOption::White,
				"%s",
				displayText.c_str()
			);
		}

		// 全文表示後に進行を促すEnter表示（サイン波で明暗点滅）
		bool isAllTextDisplayed =
			(mCurrentLine >= 0 &&
				mCurrentLine < static_cast<int>(mMessages.size()) &&
				mDisplayByteCount >= static_cast<int>(mMessages[mCurrentLine].size()));

		if (isAllTextDisplayed && !mIsTransitioning)
		{
			int pulse = static_cast<int>(180 + 75 * sinf(mWaitTimer * 8.0f));

			if (pulse < 0) pulse = 0;
			if (pulse > 255) pulse = 255;

			unsigned int enterColor = GetColor(pulse, pulse, pulse);

			Master::mpGameManager->GetFontManager()->DrawDotString(
				ScreenSize::ScrrenWidth - 200,
				ScreenSize::ScrrenHeight - 80,
				36,
				enterColor,
				"%s",
				"Enter"
			);
		}
	}

	// シーン切り替え用の全画面フェードアウト描画
	if (mIsTransitioning)
	{
		int fadeAlpha = (mTransitionTimer * 255) / 60;

		if (fadeAlpha > 255)
		{
			fadeAlpha = 255;
		}

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
		DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	Scene::Update();
}

void WinResultScene::Finalize()
{
	// 読み込んだ画像ハンドルの破棄と無効化処理
	if (mPortraitBoxHandle != -1)
	{
		DeleteGraph(mPortraitBoxHandle);
		mPortraitBoxHandle = -1;
	}
	if (mTextBoxHandle != -1)
	{
		DeleteGraph(mTextBoxHandle);
		mTextBoxHandle = -1;
	}
	for (int i = 0; i < 3; ++i)
	{
		if (mFullBodyHandles[i] != -1)
		{
			DeleteGraph(mFullBodyHandles[i]);
			mFullBodyHandles[i] = -1;
		}
	}
}