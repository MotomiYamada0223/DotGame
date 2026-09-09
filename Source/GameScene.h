#pragma once
#include "Scene.h"
#include "TutorialTextManager.h"

class GameScene : public Scene
{

public:

	GameScene();

	virtual ~GameScene();

	// 初期化
	virtual void Initialize() override;
	// 更新
	virtual void Update() override;
	// 描画
	virtual void Draw() override;
	// 終了処理
	virtual void Finalize() override;
private:
	
	// Tutorialマネージャーを呼び出すための処理
	TutorialTextManager mTutorialText;
};
