#pragma once
#include "Scene.h"
#include "TutorialTextManager.h"

// 前方宣言
class Player;

class GameScene : public Scene
{

public:

	GameScene();

	virtual ~GameScene();

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Finalize() override;

private:
	TutorialTextManager mTutorialText;

	int spawnTimer; 
	Player* mpPlayer; // プレイヤーの座標を参照するために保持
	
};
