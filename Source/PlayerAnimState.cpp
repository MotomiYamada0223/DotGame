#include "PlayerAnimState.h"

// コンストラクタ
PlayerAnimState::PlayerAnimState()
	: mState(State::Idle)
	, mPlayType(PlayType::Loop)
	, mCurrentFrame(0)
	, mFrameTimer(0)
	, mStopFrame(0)
	, mIsFinished(false)
{
}

// アニメーションの状態を変更する
void PlayerAnimState::SetState(State state)
{
	// 現在と同じ状態ならリセットしない
	// 毎フレームSetStateが呼ばれてもアニメーションが最初に戻らないようにする
	if (mState == state)
	{
		return;
	}

	// 状態を変更
	mState = state;

	// 状態が変わったのでアニメーションを最初から再生する
	mCurrentFrame = 0;
	mFrameTimer = 0;
	mIsFinished = false;
}

// 待機アニメーション
void PlayerAnimState::SetIdle()
{
	SetState(State::Idle);

	mPlayType = PlayType::Loop;
	mStopFrame = 0;
}

// 歩きアニメーション
void PlayerAnimState::SetWalk()
{
	SetState(State::Walk);

	mPlayType = PlayType::Loop;
	mStopFrame = 0;
}

// ジャンプアニメーション
void PlayerAnimState::SetJump()
{
	SetState(State::Jump);

	mPlayType = PlayType::StopFrame;
	mStopFrame = 2; // 3枚目で停止
}

// 攻撃アニメーション
void PlayerAnimState::SetAttack()
{
	SetState(State::Attack);

	mPlayType = PlayType::Once;
	mStopFrame = TOTAL_FRAMES - 1;
}

// アニメーションを1フレーム進める
void PlayerAnimState::Update()
{
	// アニメーションが終了している場合は更新しない
	if (mIsFinished)
	{
		return;
	}

	// フレーム切り替え用のタイマーを進める
	mFrameTimer++;

	// 指定した時間が経過するまでは同じフレームを表示
	if (mFrameTimer < FRAME_INTERVAL)
	{
		return;
	}

	// フレーム切り替えのタイミングなのでタイマーをリセット
	mFrameTimer = 0;

	// ループ再生
	if (mPlayType == PlayType::Loop)
	{
		mCurrentFrame++;

		// 最後まで行ったら最初に戻る
		if (mCurrentFrame >= TOTAL_FRAMES)
		{
			mCurrentFrame = 0;
		}

		return;
	}

	// 一度だけ再生
	if (mPlayType == PlayType::Once)
	{
		mCurrentFrame++;

		// 最後のフレームまで再生したら停止
		if (mCurrentFrame >= TOTAL_FRAMES)
		{
			mCurrentFrame = TOTAL_FRAMES - 1;
			mIsFinished = true;
		}

		return;
	}

	// 指定したフレームで停止
	if (mPlayType == PlayType::StopFrame)
	{
		if (mCurrentFrame < mStopFrame)
		{
			mCurrentFrame++;
		}

		// 指定したフレームに到達したら停止
		if (mCurrentFrame >= mStopFrame)
		{
			mCurrentFrame = mStopFrame;
			mIsFinished = true;
		}

		return;
	}
}