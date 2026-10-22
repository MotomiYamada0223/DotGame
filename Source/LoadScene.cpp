#include "LoadScene.h"
#include <DxLib.h>

LoadScene::LoadScene()
	:Scene()
{

}

LoadScene::~LoadScene()
{

}

void LoadScene::Initialize()
{

}

void LoadScene::Draw()
{
	DrawString(0, 0, "デバッグ用、ここはLoad", GetColor(255, 255, 255));
	Scene::Draw();
}

void LoadScene::Update()
{
	Scene::Update();
}

void LoadScene::Finalize()
{

}