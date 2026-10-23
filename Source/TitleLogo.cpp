#include "TitleLogo.h"
#include "Utility.h"
#include "GameConstants.h"


#include <stdint.h>
#include <math.h>

TitleLogo::TitleLogo()
	: mnBackGroundHandle(-1)
	, mnScrollCloudHandle(-1)
	, mnLogoHandle(-1)
	, mfScrollCloudX(0.0f)
	, mnMenuCursor(0)
	, mLogoPieces()
	, mfAnimtionTimer(0.0f)
	, mfLogoY(ScreenSize::CenterY)
	, mfCloudX(0.0f)
	, mnCloudHeight(0)
	, mnCloudWidth(0)
{
	mnBackGroundHandle = LoadGraph(BackGroundGraphPath::TitleBackground.c_str());
	mnScrollCloudHandle = LoadGraph(BackGroundGraphPath::TitleScrollCloud.c_str());
	mnLogoHandle = LoadGraph(BackGroundGraphPath::TitleLogoGraphs.c_str());


	if (mnBackGroundHandle == -1)
	{
		printfDx("背景画像がありません。");
	}
	if (mnScrollCloudHandle == -1)
	{
		printfDx("雲画像がありません。");
	}
	if (mnLogoHandle == -1)
	{
		printfDx("ロゴ画像がありません。");
	}

	// 雲画像の大きさを取得
	GetGraphSize(
		mnScrollCloudHandle,
		&mnCloudWidth,
		&mnCloudHeight);
}

TitleLogo::~TitleLogo()
{
	if (mnBackGroundHandle != -1)
	{
		DeleteGraph(mnBackGroundHandle);
		mnBackGroundHandle = -1;
	}
	if (mnScrollCloudHandle != -1)
	{
		DeleteGraph(mnScrollCloudHandle);
		mnScrollCloudHandle = -1;
	}
	if (mnLogoHandle != -1)
	{
		DeleteGraph(mnLogoHandle);
		mnLogoHandle = -1;
	}
}


void TitleLogo::Update()
{

	float scale = 2.0f;
	int cloudScaledWidth = static_cast<int>(mnCloudWidth * scale);
	mfCloudX -= 1.0f;
	if (mfCloudX <= -cloudScaledWidth)
	{
		mfCloudX += cloudScaledWidth;
	}

	UpdateLogoAnimation();
}


void TitleLogo::Draw()
{
	if (mnBackGroundHandle != -1)
	{
		DrawRotaGraph(
			ScreenSize::CenterX,
			ScreenSize::CenterY,
			1.0f,
			0.0f, 
			mnBackGroundHandle,
			TRUE);
	}

	DrawCloud();

	if (mnLogoHandle != -1)
	{
		DrawRotaGraph(
			ScreenSize::CenterX,
			static_cast<int>(mfLogoY) - LogoLowY,
			LogoExrate,
			0.0f, 
			mnLogoHandle,
			TRUE);
	}
}

void TitleLogo::DrawCloud()
{
	float scale = 2.0f;
	int bgScaledWidth = static_cast<int>(mnCloudWidth);
	int bgScaledHeight = static_cast<int>(mnCloudHeight * scale);
	int cloudScaledWidth = static_cast<int>(mnCloudWidth * scale);

	int bgX = (ScreenSize::ScrrenWidth - bgScaledWidth) / 2;
	int bgY = CloudY;



	if (mnScrollCloudHandle != -1)
	{
		int c1X = bgX + static_cast<int>(mfCloudX);
		DrawExtendGraph(
			c1X,
			bgY,
			c1X + cloudScaledWidth,
			bgY + bgScaledHeight,
			mnScrollCloudHandle,
			TRUE
		);


		DrawExtendGraph(c1X + cloudScaledWidth,
			bgY,
			c1X + cloudScaledWidth * 2,
			bgY + bgScaledHeight,
			mnScrollCloudHandle,
			TRUE
		);
	}
}


void TitleLogo::UpdateLogoAnimation()
{
	// アニメーション時間を進める
	mfAnimtionTimer += LogoMoveSpeed;

	// ロゴの基本となるY座標を設定する
	float baseY = ScreenSize::CenterY;

	// sin波を利用してロゴの目標Y座標を上下
	mfLogoY =
		baseY +
		sinf(mfAnimtionTimer) *
		LogoSwingSize;
}