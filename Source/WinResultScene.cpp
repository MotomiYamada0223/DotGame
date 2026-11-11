#include "WinResultScene.h"
#include "InputManager.h"
#include "Master.h"
#include "GameConstants.h"

WinResultScene::WinResultScene()
	:Scene()
{

}

WinResultScene::~WinResultScene()
{

}

void WinResultScene::Initialize()
{
	mPortraitBoxHandle = LoadGraph("Resource/Image/AngelPortraitBox.png");
	mTextBoxHandle = LoadGraph("Resource/Image/AngelTextBox.png");


	// 0: 目閉じ, 1: 目開き・口閉じ, 2: 目開き・口開き
	mFullBodyHandles[0] = LoadGraph("Resource/Image/八頭身天使モーション/ノーマル/angel_fullbody_closed.png");
	mFullBodyHandles[1] = LoadGraph("Resource/Image/八頭身天使モーション/ノーマル/angel_fullbody_open.png");
	mFullBodyHandles[2] = LoadGraph("Resource/Image/八頭身天使モーション/ノーマル/angel_fullbody_talk.png");


	// 画面右端の外側からスタートして中央へスライド
	mAngelX = static_cast<float>(ScreenSize::ScrrenWidth) + 300.0f;
	mAngelY = 635.0f;
	mHasArrived = false;
	mArriveWaitTimer = 0;

	mCurrentState = 1;
	mTalkTimer = 0;
	mBlinkTimer = 0;
	mIsBlinking = false;
	mBlinkInterval = 180 + GetRand(60);




	mMessages.clear();
	mMessages.push_back("ステージクリアおめでとう！\nよく頑張ったわね！");
	char deathMessage[256];
	sprintf_s(
		deathMessage,
		"あなたは今回のステージで%d回死亡したわ",
		Master::gDeathCounts[Master::gCurrentStageIndex]// 現在のステージでの死亡回数
	);
	mMessages.push_back(deathMessage);



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
	if (mIsTransitioning)
	{
		mTransitionTimer++;
		if (mTransitionTimer >= 60)
		{
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_STAGESELECT);
		}
		return;
	}

	// 瞬きアニメーション（スライド中も会話中も動作）
	if (mIsBlinking)
	{
		mBlinkTimer++;
		if (mBlinkTimer >= 6)
		{
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
			mIsBlinking = true;
			mBlinkTimer = 0;
		}
	}

	// フェーズ1：右から中央へスライドして止まるまでテキストを進めない
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

	// フェーズ2：中央で停止した後にセリフを開始
	const std::string& currentText = mMessages[mCurrentLine];
	bool isTyping = (mDisplayByteCount < static_cast<int>(currentText.size()));

	if (!mIsBlinking)
	{
		if (isTyping)
		{
			mTalkTimer++;
			if (mTalkTimer >= 12)
			{
				mTalkTimer = 0;
				mCurrentState = (mCurrentState == 1) ? 2 : 1;
			}
		}
		else
		{
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
			if (mDisplayByteCount > static_cast<int>(currentText.size()))
			{
				mDisplayByteCount = static_cast<int>(currentText.size());
			}
		}

		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) || InputManager::CheckDownKey(KEY_INPUT_SPACE))
		{
			mDisplayByteCount = static_cast<int>(currentText.size());
		}
	}
	else
	{
		mWaitTimer += dt;
		if (InputManager::CheckDownKey(KEY_INPUT_RETURN) || InputManager::CheckDownKey(KEY_INPUT_SPACE) || mWaitTimer >= 2.5f)
		{
			mCurrentLine++;
			if (mCurrentLine >= static_cast<int>(mMessages.size()))
			{
				mIsTransitioning = true;
				mTransitionTimer = 0;
				mCurrentLine = static_cast<int>(mMessages.size()) - 1;
			}
			else
			{
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
	// 真っ暗な背景
	DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);

	// 8頭身の天使キャラクター
	if (mCurrentState >= 0 && mCurrentState < 3 && mFullBodyHandles[mCurrentState] != -1)
	{
		DrawRotaGraph(static_cast<int>(mAngelX), static_cast<int>(mAngelY), 0.72, 0.0, mFullBodyHandles[mCurrentState], TRUE);
	}

	// 中央に止まってからテキストボックスとセリフを表示
	if (mHasArrived)
	{
		int saintX = 580;
		int saintY = 100;

		if (mPortraitBoxHandle != -1)
		{
			DrawGraph(saintX - 95, saintY - 95, mPortraitBoxHandle, TRUE);
		}

		if (mTextBoxHandle != -1)
		{
			DrawGraph(680, 5, mTextBoxHandle, TRUE);
		}

		// セリフの描画
		if (mCurrentLine >= 0 && mCurrentLine < static_cast<int>(mMessages.size()))
		{
			std::string displayText = mMessages[mCurrentLine].substr(0, mDisplayByteCount);

			Master::mpGameManager->GetFontManager()->DrawDotString(
				720,
				25,
				50,
				GetColor(255, 255, 0),
				"%s",
				displayText.c_str()
			);
		}

		// テキストがすべて出た後、画面右下に「Enter」を表示
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

	// フェードアウト
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
	Scene::Draw();
}



void WinResultScene::Finalize()
{
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