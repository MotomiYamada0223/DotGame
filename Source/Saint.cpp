#include "Saint.h"
#include "DxLib.h"
#include "Utility.h"

Saint::Saint(VECTOR initPos)
	: Object2D(initPos)
{
	SetTag(Object2D::Tag::FrontMap);

	graphHandles[0] = LoadGraph("Resource/Image/saint_close1.png"); // 目と口を閉じたやつ（瞬き）
	graphHandles[1] = LoadGraph("Resource/Image/saint_close2.png"); // 目を開けて口を閉じたやつ
	graphHandles[2] = LoadGraph("Resource/Image/saint_open2.png"); // 両方空いてるやつ

	currentState = 1; // 初期状態
	
	talkTimer = 0;
	blinkTimer = 0;
	isBlinking = false;
	blinkInterval = 180 + GetRand(60); // 次の瞬きまで 3〜4秒(180〜240フレーム)
}

Saint::~Saint()
{
	for (int i = 0; i < 3; ++i)
	{
		if (graphHandles[i] != -1)
		{
			DeleteGraph(graphHandles[i]);
			graphHandles[i] = -1;
		}
	}
}

void Saint::Update()
{
	// 瞬き処理
	
	if (isBlinking)
	{
		blinkTimer++;
		// 0.1秒(6フレーム)ほど目を閉じたものを表示
		
		if (blinkTimer >= 6) 
		{
			isBlinking = false;
			blinkTimer = 0;
			// 次の瞬きは3〜4秒後
			
			blinkInterval = 180 + GetRand(60); 
		}
		
		currentState = 0; // 目閉・口閉
	}
	else
	{
		// 瞬きまでの時間をカウント
		
		blinkTimer++;
		if (blinkTimer >= blinkInterval)
		{
			isBlinking = true;
			blinkTimer = 0;
		}

		// しゃべる処理（瞬きしていない時だけ口を動かす）
		
		talkTimer++;

		// 0.3秒(18フレーム)間隔で切り替え
		
		if (talkTimer >= 18) 
		{
			talkTimer = 0;

			// 目開口閉(1) と 両方開(2) を交互に切り替える
			
			if (currentState == 1) 
			{
				currentState = 2;
			}
			else 
			{
				currentState = 1;
			}
		}
	}

	Object2D::Update();
}

void Saint::Draw()
{
	if (currentState >= 0 && currentState < 3)
	{
		int handle = graphHandles[currentState];
		if (handle != -1)
		{
			// 中心基準で 3倍(3.0) のサイズに拡大して描画する
			
			DrawRotaGraph(static_cast<int>(mvPosition.x), static_cast<int>(mvPosition.y), 3.0, 0.0, handle, TRUE);
		}
	}
}
