#pragma once
#include "Scene.h"

class GameScene : public Scene
{

public:

	GameScene();

	virtual ~GameScene();

	// ‰Šú‰»
	virtual void Initialize() override;
	// XV
	virtual void Update() override;
	// •`‰æ
	virtual void Draw() override;
	// I—¹ˆ—
	virtual void Finalize() override;
private:
};
