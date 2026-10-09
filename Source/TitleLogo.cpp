#include "TitleLogo.h"
#include "Utility.h"
#include "GameConstants.h"


#include <stdint.h>
#include <math.h>

TitleLogo::TitleLogo()
	: mnBackGroundHandle(-1)
	, mnfScrollCloudHandle(-1)
	, mnLogoHandle(-1)
	, mfScrollCloudX(0.0f)
	, mnMenuCursor(0)
	, mLogoPieces()
	, mfAnimtionTimer(0.0f)
	, mfLogoY(ScreenSize::CenterY)
{
	mnBackGroundHandle = LoadGraph("Resource/SceneBackground/background_noclouds.png");
	mnfScrollCloudHandle = LoadGraph("Resource/SceneBackground/cloud_seamless.png");
	mnLogoHandle = LoadGraph("Resource/LogoImage/title_logo.png");


	if (mnBackGroundHandle == -1)
	{
		printfDx("背景画像がありません。");
	}
	if (mnfScrollCloudHandle == -1)
	{
		printfDx("雲画像がありません。");
	}
	if (mnLogoHandle == -1)
	{
		printfDx("ロゴ画像がありません。");
	}
}

TitleLogo::~TitleLogo()
{
	if (mnBackGroundHandle != -1)
	{
		DeleteGraph(mnBackGroundHandle);
		mnBackGroundHandle = -1;
	}
	if (mnfScrollCloudHandle != -1)
	{
		DeleteGraph(mnfScrollCloudHandle);
		mnfScrollCloudHandle = -1;
	}
	if (mnLogoHandle != -1)
	{
		DeleteGraph(mnLogoHandle);
		mnLogoHandle = -1;
	}
}


void TitleLogo::Update()
{
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

	if (mnBackGroundHandle != -1)
	{
		DrawRotaGraph(
			ScreenSize::CenterX,
			static_cast<int>(mfLogoY),
			LogoExrate,
			0.0f, 
			mnLogoHandle,
			TRUE);
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