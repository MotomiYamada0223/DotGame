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
		switch (mnSelectIndex)
		{
		case 0:
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TYPE::SCENE_STAGESELECT);
			break;

		case 1:
			 // まだ実装してないから、SE出しても良い
			break;

		case 2:
			DxLib_End();
			break;

		default:
			break;

		}
	}

	mTitleLogo.Update();

	// 基底クラスの更新処理を呼びだす
	Scene::Update();
}


// キー入力で選択
void TitleScene::UpdateSelectString()
{
	if (InputManager::CheckDownKey(KEY_INPUT_W) || InputManager::CheckDownKey(KEY_INPUT_UP))
	{
		mnSelectIndex--;
		if (mnSelectIndex < 0)
		{
			mnSelectIndex = MAX_SELECT-1;
		}
	}
	if (InputManager::CheckDownKey(KEY_INPUT_S) || InputManager::CheckDownKey(KEY_INPUT_DOWN))
	{
		mnSelectIndex++;
		if (mnSelectIndex >= MAX_SELECT)
		{
			mnSelectIndex = 0;
		}
	}
}

void TitleScene::DrawSelectString()
{
	std::string selectList[] =
	{
		"Play",
		"Continue",
		"Exit"
	};

	// 参考演算子で選択中のを見やすく表示
	int stringX = ScreenSize::CenterX - 50;
	int stringY = ScreenSize::CenterY + 200;
	int lineSpacing = 80; // 項目ごとの間隔
	int fontSize = 70;

	int SelectNow = mnSelectIndex; 
	for (int i = 0; i < MAX_SELECT; i++) // 表示するため
	{
		int posY = stringY + (i - 1) * lineSpacing;

		// 選択中か、それ以外かの処理を三項演算子でやってる
		int Color = (i == SelectNow) ? GetColor(255, 255, 100) : GetColor(255, 100, 100);
		Master::mpGameManager->GetFontManager()->DrawDotString(
			stringX, posY, fontSize,
			Color, "%s", selectList[i].c_str());

		if (i == SelectNow)
		{
			Master::mpGameManager->GetFontManager()->DrawDotString(
				stringX - 40, posY, fontSize,
				Color, ">");
		}
	}
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
