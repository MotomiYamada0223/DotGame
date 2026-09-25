#pragma once
#include "Scene.h"

class LoseResultScene : public Scene
{
public:
	LoseResultScene();
	~LoseResultScene();

	void Initialize();
	void Draw();
	void Update();
	void Finalize();

private:
};