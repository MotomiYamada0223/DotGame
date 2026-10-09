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
	, mfAnimtionTimer(0)
	, mfLogoY(LogoInitialY)
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
	// ロゴの移動速度を保持する
	static float velocity = 0.0f;

	// アニメーション時間を進める
	mfAnimtionTimer += LogoMoveSpeed;

	// ロゴの基本となるY座標を設定する
	float baseY =
		(Utility::SCREEN_HEIGHT / 2) +
		LogoBaseYOffset;

	// sin波を利用してロゴの目標Y座標を上下に変化させる
	float targetY =
		baseY +
		sinf(mfAnimtionTimer) *
		LogoSwingSize;

	// バネ演出に使用する各パラメータを取得する
	float gravity = Gravity;       // 重力の強さ
	float power = SpringPower;     // バネの引き戻す強さ
	float damping = Damping;       // 速度の減衰率

	// 重力を加えてロゴの速度を変化させる
	velocity += gravity;

	// 現在位置と目標位置の差からバネの力を計算する
	float force = (targetY - mfLogoY) * power;

	// バネの力を速度に加える
	velocity += force;

	// 速度を減衰させて動きを調整する
	velocity *= damping;

	// 計算した速度を現在のY座標に反映する
	mfLogoY += velocity;

	// ロゴの速度と目標位置との差が十分に小さい場合、位置を補正する
	if (fabs(velocity) < VelocityStop &&
		fabs(targetY - mfLogoY) <
		TargetStopDistance)
	{
		// ロゴを目標位置に合わせる
		mfLogoY = targetY;

		// 速度をゼロにする
		velocity = 0.0f;
	}
}