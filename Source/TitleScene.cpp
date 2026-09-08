#include "TitleScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"


TitleScene::TitleScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
{
}

TitleScene::~TitleScene()
{

}

void TitleScene::Initialize()
{
	// タイトルロゴのクラスの作成
	// プレイヤーの生成 などをここで行う
	// タイトル画面で必要なオブジェクトをここで生成する

	// BGM再生
	//Master::mpSoundManager->PlayBGM(SoundManager::BGM_TITLE);
}


void TitleScene::Update()
{
	// Enterでゲーム画面へ
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::SCENE_GAME);
	}

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}


void TitleScene::Draw()
{

	DrawFormatStringToHandle(500, 500, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "タイトル　才能の原石\n\n Enterでゲームシーン");

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void TitleScene::Finalize()
{
	// BGM停止
	//Master::mpSoundManager->StopBGM();
}