#pragma once
#include "Scene.h"
#include <vector>
#include <string>

/// <summary>
/// 勝利した後の演出をするシーンクラス
/// </summary>
class WinResultScene : public Scene
{
public:
	WinResultScene();
	~WinResultScene();

	void Initialize();
	void Draw();
	void Update();
	void Finalize();

private:
	int mPortraitBoxHandle;
	int mTextBoxHandle;

	std::vector<std::string> mMessages;
	int mFullBodyHandles[3];
	float mAngelX;
	float mAngelY;
	bool mHasArrived;
	int mArriveWaitTimer;

	int mCurrentState;
	int mTalkTimer;
	int mBlinkTimer;
	int mBlinkInterval;
	bool mIsBlinking;

	int mCurrentLine;
	int mDisplayByteCount;
	float mCharTimer;
	float mCharSpeed;
	float mWaitTimer;

	bool mIsTransitioning;
	int mTransitionTimer;
};