#include "TitleScene.h"
#include "Utility.h" // 呼び出すと、SCREEN_WIDTHとかを使える
#include "DxLib.h"
#include "Master.h"
#include "inputManager.h"


TitleScene::TitleScene()
	: Scene()     // 基底クラスのコンストラクタを呼び出しておく
	, mTitleLogo()
	, mnSelectIndex(0)
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
	UpdateSelectString();
	// Enterでゲーム画面へ
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		if (mnSelectIndex == 0)
		{
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TYPE::SCENE_STAGESELECT);
		}
		else
		{
			DxLib_End();
		}
	}

	mTitleLogo.Update();

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}

void TitleScene::UpdateSelectString()
{
	if (InputManager::CheckDownKey(KEY_INPUT_W) || InputManager::CheckDownKey(KEY_INPUT_UP))
	{
		mnSelectIndex--;
		if (mnSelectIndex < 0)
		{
			mnSelectIndex = 1; // 項目数に合わせて調整（全2項目なら 1）
		}
	}
	if (InputManager::CheckDownKey(KEY_INPUT_S) || InputManager::CheckDownKey(KEY_INPUT_DOWN))
	{
		mnSelectIndex++;
		if (mnSelectIndex > 1)
		{
			mnSelectIndex = 0; // 0に戻る
		}
	}
}

void TitleScene::DrawSelectString()
{
	// メニュー項目の座標基準
	int menuX = ScreenSize::CenterX;
	int menuStartY = ScreenSize::CenterY + 100;
	int lineSpacing = 80; // 項目ごとの間隔
	int fontSize = 70;

	if (mnSelectIndex == 0)
	{
		Master::mpGameManager->GetFontManager()->DrawDotString(menuX - 50, menuStartY, 80, GetColor(255, 255, 255), ">");
	}
	Master::mpGameManager->GetFontManager()->DrawDotString(
		menuX, menuStartY, fontSize,
		GetColor(255, 255, 255), "Play");
	if (mnSelectIndex == 1)
	{
		Master::mpGameManager->GetFontManager()->DrawDotString(menuX - 50, menuStartY + lineSpacing, 80, GetColor(255, 255, 255), ">");
	}
	Master::mpGameManager->GetFontManager()->DrawDotString(
		menuX, menuStartY + lineSpacing, fontSize,
		GetColor(255, 255, 255), "Continue");
}

void TitleScene::Draw()
{
		Master::mpGameManager->GetFontManager()->DrawDotString(
			500,
			500,
			100,
			GetColor(255, 255, 255),
			"タイトル　才能の原石\n\n Enterでゲームシーン"
		);

		mTitleLogo.Draw();

		DrawSelectString();

	// 基底クラスの更新処理を呼びだす
	Scene::Draw();
}


void TitleScene::Finalize()
{
	// BGM停止
	//Master::mpSoundManager->StopBGM();
}
