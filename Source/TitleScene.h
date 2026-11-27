#pragma once
#include "Scene.h" // シーン継承したいからインクルード
#include "TitleLogo.h"
#include "FadeController.h"


class TitleScene : public Scene
{

public:
	TitleScene();
	virtual ~TitleScene();

	// 初期化
	virtual void Initialize() override;
	// 更新
	virtual void Update() override;
	// 描画
	virtual void Draw() override;
	// 終了処理
	virtual void Finalize() override;

	void DrawSelectString();
	void UpdateSelectString();

private:
	TitleLogo mTitleLogo;
	FadeController mFade;

	int mnSelectIndex;
	const int MAX_SELECT = 2;
	bool mbSceneChangeRequested;
	bool mbDirectStage3Requested = false;
};

