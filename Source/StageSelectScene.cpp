#include "StageSelectScene.h"
#include "DxLib.h"
#include "Utility.h"
#include "InputManager.h"
#include "Master.h"
#include "SceneManager.h"
#include <math.h> // sqrtfを使用するため追加

StageSelectScene::StageSelectScene()
	: Scene()
{
	mapGraphHandle = -1;
	playerGraphHandle = -1;
	playerX = 0.0f;
	playerY = 0.0f;
	playerSpeed = 5.0f;
	targetX = 0.0f;
	targetY = 0.0f;
	isMoving = false;
	lastSignal = "None";
}

StageSelectScene::~StageSelectScene()
{
}

void StageSelectScene::Initialize()
{
	// 背景画像の読み込み
	
	mapGraphHandle = LoadGraph("Resource/Image/SampleStageMap.png");

	// プレイヤーのスケルトンの画像の読み込みと初期位置の設定	
	playerGraphHandle = LoadGraph("Resource/Image/SampleSkeleton.png");
	
	// 初期座標	
	playerX = -40.0f;
	playerY = 240.0f;
	
	// 最初の目標座標	
	targetX = 250.0f;
	targetY = 240.0f;
	isMoving = true; // 最初の目標に向かって自動で移動する
	lastSignal = "None";
}

void StageSelectScene::Update()
{
	if (isMoving)
	{
		// 目標座標に向かって移動する	
		float dx = targetX - playerX;
		float dy = targetY - playerY;
		float dist = sqrtf(dx * dx + dy * dy);

		if (dist <= playerSpeed)
		{
			// 目標に到着した
			
			playerX = targetX;
			playerY = targetY;
			isMoving = false;
			lastSignal = "None"; // 到着したら信号をリセット

			// 特定座標(250, 625)に到着した場合、すぐに次の座標(705, 625)へ向かう			
			if (playerX == 250.0f && playerY == 625.0f)
			{
				targetX = 705.0f;
				targetY = 625.0f;
				isMoving = true;
			}
			// (705, 95)に到着した場合、すぐに(1020, 95)へ向かう			
			else if (playerX == 705.0f && playerY == 95.0f)
			{
				targetX = 1020.0f;
				targetY = 95.0f;
				isMoving = true;
			}
			// (705, 960)に到着した場合、すぐに(1235, 960)へ向かう			
			else if (playerX == 705.0f && playerY == 960.0f)
			{
				targetX = 1235.0f;
				targetY = 960.0f;
				isMoving = true;
			}
			// (1595, 95)に到着した場合、すぐに(1595, 600)へ向かう			
			else if (playerX == 1595.0f && playerY == 95.0f)
			{
				targetX = 1595.0f;
				targetY = 600.0f;
				isMoving = true;
			}
		}
		else
		{	
			playerX += (dx / dist) * playerSpeed;
			playerY += (dy / dist) * playerSpeed;
		}
	}
	else
	{
		// 移動中でいるときのみ、WASDの信号を受け付ける		
		if (CheckHitKey(KEY_INPUT_W) == 1)
		{
			lastSignal = "UP";

			// 現在地が(705, 625)の時にWキーが押されたら、(705, 95)へ移動を開始する			
			if (playerX == 705.0f && playerY == 625.0f)
			{
				targetX = 705.0f;
				targetY = 95.0f;
				isMoving = true;
			}
		}
		else if (CheckHitKey(KEY_INPUT_S) == 1)
		{
			lastSignal = "DOWN";

			// 現在地が(250, 240)の時にSキーが押されたら、(250, 625)へ移動を開始する		
			if (playerX == 250.0f && playerY == 240.0f)
			{
				targetX = 250.0f;
				targetY = 625.0f;
				isMoving = true;
			}
			// 現在地が(705, 625)の時にSキーが押されたら、(705, 960)へ移動を開始する	
			else if (playerX == 705.0f && playerY == 625.0f)
			{
				targetX = 705.0f;
				targetY = 960.0f;
				isMoving = true;
			}
		}
		else if (CheckHitKey(KEY_INPUT_A) == 1)
		{
			lastSignal = "LEFT";
		}
		else if (CheckHitKey(KEY_INPUT_D) == 1)
		{
			lastSignal = "RIGHT";

			// 現在地が(1020, 95)の時にDキーが押されたら、(1595, 95)へ移動を開始する			
			if (playerX == 1020.0f && playerY == 95.0f)
			{
				targetX = 1595.0f;
				targetY = 95.0f;
				isMoving = true;
			}
		}
	}

	// Enterでゲームシーンへ
	
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN))
	{
		Master::mpSceneManager->SetNextScene(SceneManager::SCENE_GAME);
	}

	Scene::Update();
}

void StageSelectScene::Draw()
{
	if (mapGraphHandle != -1)
	{
		// 背景を画面全体に拡大描画
		
		DrawExtendGraph(0, 0, Utility::SCREEN_WIDTH, Utility::SCREEN_HEIGHT, mapGraphHandle, TRUE);
	}

	if (playerGraphHandle != -1)
	{	
		DrawRotaGraph(static_cast<int>(playerX), static_cast<int>(playerY), 0.2, 0.0, playerGraphHandle, TRUE);
	}

	int oldFontSize = GetFontSize();
	SetFontSize(10);
	DrawFormatString(10, 10, GetColor(255, 255, 0), "PlayerPos X:%.1f Y:%.1f", playerX, playerY);
	DrawFormatString(10, 25, GetColor(255, 150, 150), "Input Signal: %s", lastSignal.c_str());
	DrawFormatString(10, 40, GetColor(150, 255, 150), "Status: %s", isMoving ? "Moving..." : "Waiting");
	SetFontSize(oldFontSize);

	DrawFormatStringToHandle(50, 60, GetColor(255, 255, 255),
		Master::mpFontManager->GetDotFont(), "Stage Select Scene\nPress Enter to Game");

	Scene::Draw();
}

void StageSelectScene::Finalize()
{
	if (playerGraphHandle != -1)
	{
		DeleteGraph(playerGraphHandle);
		playerGraphHandle = -1;
	}

	if (mapGraphHandle != -1)
	{
		DeleteGraph(mapGraphHandle);
		mapGraphHandle = -1;
	}
}
