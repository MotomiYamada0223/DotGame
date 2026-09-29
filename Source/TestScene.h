#pragma once
#include "UnitStatus.h"
#include "Scene.h"
#include "TutorialTextManager.h"
#include"BlockMap.h"

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

private:
	TutorialTextManager mTutorialText;

	int spawnTimer; 
	Player* mpPlayer;
	GameProgress mProgress; // プレイヤーの座標を参照するために保持
	BlockMap mBlockMap;

	bool mbIsLoaded = false; // マップがロードされたかどうかのフラグ
};
