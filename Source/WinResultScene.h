#pragma once
#include "Scene.h"

/// <summary>
/// 勝利した後の演出をするシーンクラス
/// </summary>
class WinResultScene : public Scene
{
public:
	WinResultScene();
	~WinResultScene();

	void Initialize();
	void Draw();
	void Update();
	void Finalize();

private:

};