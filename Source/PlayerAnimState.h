#pragma once

class PlayerAnimState
{
public:
	enum class State
	{
		Idle,
		Walk,
		Jump,
		Attack
	};

	enum class PlayType
	{
		Loop,
		Once,
		StopFrame
	};

	PlayerAnimState();

	void SetState(State state);

	void SetIdle();
	void SetWalk();
	void SetJump();
	void SetAttack();

	void Update();

	State GetState() const { return mState; }

	int GetCurrentFrame() const { return mCurrentFrame; }
	int GetFrameTimer() const { return mFrameTimer; }

	bool IsFinished() const { return mIsFinished; }

	// プレイヤーのアニメーションで使う変数
	// アニメーションさせる画像情報
	static const int FRAME_WIDTH = 128;
	static const int FRAME_HEIGHT = 128;
	static const int TOTAL_FRAMES = 4;
	static const int FRAME_INTERVAL = 8;

	// アニメーションさせる時に映す基準位置
	static const int IDLE_POS = 0;
	static const int WALK_POS = 128;
	static const int JUMP_POS = 256;
	static const int ATTACK_POS = 384;

private:
	State mState;
	PlayType mPlayType;

	int mCurrentFrame;
	int mFrameTimer;

	int mStopFrame;

	bool mIsFinished;
};