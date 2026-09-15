#include "DxLib.h"
#include "Texture.h"
#include "TextureAnimation.h"
#include "Collision.h"
#include "SceneManager.h"
#include "Master.h"
#include "ObjectManager.h"
#include "FontManager.h"
#include "Scene.h"
#include "Utility.h"

/*
 @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

// Master クラスの静的メンバ変数定義
GameManager* Master::mpGameManager = new GameManager();


/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// ウインドウモードで起動  falseにすると全画面
	ChangeWindowMode(true); 
	SetBackgroundColor(50, 50, 50);
	// サイズを合わせる
	SetGraphMode(Utility::SCREEN_WIDTH, Utility::SCREEN_HEIGHT, 0);  // モニター解像度に合わせる

	// DXライブラリ初期化
	if(DxLib_Init() == -1)
	{
		return -1;
	}

	// Manager関係の初期化
	Master::mpGameManager->Initialize();

	// 描画先画面を裏画面に設定する
	SetDrawScreen(DX_SCREEN_BACK);


	// ゲームのメインループ
	// ProcessMessage() == 0  ウィンドウの☓ボタン押されていないかどうか
	// CheckHitKey(KEY_INPUT_ESCAPE) == 0  エスケープキーが押されていないかどうか
	int animationCounter = 0;
	int textureCurrentNum = 0;
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		//画面を初期化する
		ClearDrawScreen();

		int time = GetNowCount();

		// Managerクラスの更新
		Master::mpGameManager->Update();


		// SceneManagerの描画
		Master::mpGameManager->GetSceneManager()->Draw();


		// 裏画面の内容を表画面に映す
		ScreenFlip();

		// 17ミリ秒（秒間約60フレームだった場合の１フレーム当たりの経過時間）
		// 経過するまでここで待つ
		while (GetNowCount() - time < 17)
		{
			// 待つだけなので何も処理はしない
		}

		// 削除する必要のあるオブジェクトがあれば削除する
		Master::mpGameManager->GetSceneManager()
			->GetCurrentScene()
			->GetObjectManager()
			->DeleteAll2DIfNeeded();


		// ループする直前にシーン遷移チェックをいれておく
		Master::mpGameManager->GetSceneManager()->ChangeSceneIfNeeded();
	}

	// 終了処理
	// Manager関係の終了処理
	Master::mpGameManager->Finalize();
	delete Master::mpGameManager;


	// DXライブラリ使用の終了
	DxLib_End();

	// ソフトの終了
	return 0;
}