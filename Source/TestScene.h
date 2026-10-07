#pragma once
#include "UnitStatus.h"
#include "Scene.h"
#include "TutorialTextManager.h"
#include"BlockMap.h"
#include "Background.h"
#include "DrawFrame.h"
#include "Saint.h"

// 前方宣言
class Player;

class TestScene : public Scene
{

public:

	TestScene();

	virtual ~TestScene();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Finalize() override;

	void SetTextUpdate(); // チュートリアルテキストの更新

private:
	TutorialTextManager mTutorialText;

	int spawnTimer; 
	Player* mpPlayer;
	GameProgress mProgress; // プレイヤーの座標を参照するために保持
	BlockMap mBlockMap;
	Background mBackground; // スクロール背景
	DrawFrame mDrawFrame; // フレーム描画用
	Saint mSaint; // 天使のキャラクター画像

	bool mbIsLoaded = false; // マップがロードされたかどうかのフラグ
	bool mbWasPlayerDead = false; // 前フレームのプレイヤーの死亡状態を保持するフラグ
};
