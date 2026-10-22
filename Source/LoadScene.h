#pragma once
#include "Scene.h"

class LoadScene : public Scene
{
private:


public:
	LoadScene();
	~LoadScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:

};