#pragma once
#include "Scene.h"
#include "Player.h"

class LoadScene : public Scene
{
private:
	static constexpr float MAX_TIMER = 1.5f;

public:
	LoadScene();
	~LoadScene() override;

	void Initialize() override;
	void Update() override;
	void Draw();
	void Finalize() override;

private:
	float mfTimer;
	bool mbFlag;
	int mPrevTime;

	int mnHandle;
};