#pragma once

class FadeController
{
public:

	// フェードの種類の定義
	enum class FadeState
	{
		Idle,
		FadeOut,
		FadeIn,
		Finished // フェード完了
	};

public:

	FadeController();
	~FadeController();

	// フェード開始
	void StartFadeOut(int fadeFrames);
	void StartFadeIn(int fadeFrames);

	void Update();
	void Draw() const;

	// 状態のアクセサ
	bool IsIdle() const { return mState == FadeState::Idle; }
	bool IsFinished() const { return mState == FadeState::Finished; }
	bool IsFadeOutFinished() const;

private:
	FadeState mState;
	int mAlpha; // 0-255
	int mnTotalFrames; // フェード全体のフレーム数
	int mnFrameCount; // 経過フレーム数
};