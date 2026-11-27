#include "Stage3BossCamera.h"
#include "BlockMap.h"
#include "Player.h"
#include "Stage3BossDragon.h"
#include "GameConstants.h"

Stage3BossCamera::Stage3BossCamera()
	: mCanvasScreenHandle(-1)
	, mbActive(false)
	, mbDrawingToCanvas(false)
	, mZoomProgress(0.0f)
	, mStartScrollX(0)
	, mStartScrollY(0)
	, mCurrentViewWidth(ScreenSize::ScrrenWidth)
	, mCurrentViewHeight(ScreenSize::ScrrenHeight)
{
}

Stage3BossCamera::~Stage3BossCamera()
{
	Finalize();
}

void Stage3BossCamera::Initialize()
{
	if (mCanvasScreenHandle == -1)
	{
		mCanvasScreenHandle = MakeScreen(BOSS_VIEW_WIDTH, BOSS_VIEW_HEIGHT, FALSE);
	}
	Reset();
}

void Stage3BossCamera::Reset()
{
	mbActive           = false;
	mbDrawingToCanvas  = false;
	mZoomProgress      = 0.0f;
	mStartScrollX      = 0;
	mStartScrollY      = 0;
	mCurrentViewWidth  = ScreenSize::ScrrenWidth;
	mCurrentViewHeight = ScreenSize::ScrrenHeight;
}

void Stage3BossCamera::Finalize()
{
	if (mCanvasScreenHandle != -1)
	{
		DeleteGraph(mCanvasScreenHandle);
		mCanvasScreenHandle = -1;
	}
	mbActive          = false;
	mbDrawingToCanvas = false;
}

void Stage3BossCamera::Update(BlockMap& blockMap, Player* player, Stage3BossDragon* boss, bool isBossFightActive)
{
	(void)player;
	(void)boss;

	if (!isBossFightActive)
	{
		mbActive           = false;
		mZoomProgress      = 0.0f;
		mCurrentViewWidth  = ScreenSize::ScrrenWidth;
		mCurrentViewHeight = ScreenSize::ScrrenHeight;
		return;
	}

	// ボス戦開始瞬間（ムービー終了直後）のスクロール位置を記録して滑らかに引きカメラへ補間開始
	if (!mbActive)
	{
		mbActive      = true;
		mZoomProgress = 0.0f;
		mStartScrollX = blockMap.GetScrollX();
		mStartScrollY = blockMap.GetScrollY();
	}

	// 約35フレームかけて滑らかに引きカメラ（2400x1350視野）へズームアウト
	if (mZoomProgress < 1.0f)
	{
		mZoomProgress += (1.0f / 35.0f);
		if (mZoomProgress > 1.0f)
		{
			mZoomProgress = 1.0f;
		}
	}

	// SmoothStep イージング
	float ease = mZoomProgress * mZoomProgress * (3.0f - 2.0f * mZoomProgress);

	// 視野サイズを 1920x1080 -> 2400x1350 へ補間
	mCurrentViewWidth  = ScreenSize::ScrrenWidth  + static_cast<int>((BOSS_VIEW_WIDTH  - ScreenSize::ScrrenWidth)  * ease);
	mCurrentViewHeight = ScreenSize::ScrrenHeight + static_cast<int>((BOSS_VIEW_HEIGHT - ScreenSize::ScrrenHeight) * ease);

	// スクロール位置をボス部屋全体が収まる (3324, 180) へ補間
	int targetScrollX = static_cast<int>(mStartScrollX + (BOSS_ROOM_SCROLL_X - mStartScrollX) * ease);
	int targetScrollY = static_cast<int>(mStartScrollY + (BOSS_ROOM_SCROLL_Y - mStartScrollY) * ease);

	// マップ画像範囲（5724 x 1692）からはみ出さないようにクランプ
	int maxScrollX = blockMap.GetCurrentWidth()  - mCurrentViewWidth;
	int maxScrollY = blockMap.GetCurrentHeight() - mCurrentViewHeight;
	if (maxScrollX < 0) maxScrollX = 0;
	if (maxScrollY < 0) maxScrollY = 0;

	if (targetScrollX < 0)          targetScrollX = 0;
	if (targetScrollX > maxScrollX) targetScrollX = maxScrollX;
	if (targetScrollY < 0)          targetScrollY = 0;
	if (targetScrollY > maxScrollY) targetScrollY = maxScrollY;

	blockMap.SetScrollX(targetScrollX);
	blockMap.SetScrollY(targetScrollY);
}

void Stage3BossCamera::BeginWorldDraw()
{
	mbDrawingToCanvas = false;
	if (!IsActive() || mCanvasScreenHandle == -1)
	{
		return;
	}

	SetDrawScreen(mCanvasScreenHandle);
	SetDrawArea(0, 0, BOSS_VIEW_WIDTH, BOSS_VIEW_HEIGHT);
	ClearDrawScreen();
	mbDrawingToCanvas = true;
}

void Stage3BossCamera::EndWorldDraw()
{
	if (!mbDrawingToCanvas)
	{
		return;
	}

	mbDrawingToCanvas = false;
	SetDrawScreen(DX_SCREEN_BACK);
	SetDrawArea(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight);

	// 広角キャンバス（mCurrentViewWidth x mCurrentViewHeight）を画面全体（1920 x 1080）へ滑らかに縮小描画
	int prevDrawMode = GetDrawMode();
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	DrawRectExtendGraph(
		0, 0,
		ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight,
		0, 0,
		mCurrentViewWidth, mCurrentViewHeight,
		mCanvasScreenHandle,
		FALSE
	);
	SetDrawMode(prevDrawMode);
}
