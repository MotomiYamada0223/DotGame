#pragma once
#include <DxLib.h>

struct LogoPiece
{
	float startX;
	float startY;
	float startAngle;
	int delay;
};

class TitleLogo
{
private:
	static constexpr float LogoExrate = 1.8f; // ƒƒS‰æ‘œ‚ÌŠg‘å—¦

	// ƒƒS
	static constexpr float LogoInitialY = -800.0f;
	static constexpr float LogoMoveSpeed = 0.05f;
	static constexpr float LogoSwingSize = 17.0f;

	// ƒƒS‚Ìƒoƒl‰‰o
	static constexpr float Gravity = 0.03f;
	static constexpr float SpringPower = 0.03f;
	static constexpr float Damping = 0.67f;
	static constexpr float VelocityStop = 0.3f;
	static constexpr float TargetStopDistance = 0.3f;
	static constexpr int LogoBaseYOffset = -150;

public:
	TitleLogo();
	~TitleLogo();
	void Update();
	void Draw();

	void UpdateLogoAnimation();

private:
	int mnBackGroundHandle;
	int mnfScrollCloudHandle;
	int mnLogoHandle;

	float mfScrollCloudX;
	int mnMenuCursor;


	// ƒƒS‚ÌƒAƒjƒ[ƒVƒ‡ƒ“
	LogoPiece mLogoPieces[60];
	float mfLogoY;
	float mfAnimtionTimer;
};