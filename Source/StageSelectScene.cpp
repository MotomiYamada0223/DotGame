#include "StageSelectScene.h"
#include "DxLib.h"
#include "Utility.h"
#include "InputManager.h"
#include "Master.h"
#include "SceneManager.h"
#include <math.h> 
#include "GameConstants.h"
#include "PlayerAnimState.h"


namespace
{
	constexpr int   kEnterAnimFrames = 60;     // 演出の長さ（フェードアウトと合わせる）
	constexpr float kEnterRiseY = 30.0f;  // 最終的に上へずらす量（px）
	constexpr float kEnterMinScale = 0.80f;  // 最終的な拡大率
}


// コンストラクタ
StageSelectScene::StageSelectScene()
	: Scene()
	, mBackgroundBox()
{
	mapGraphHandle = -1;
	for (int i = 0; i < 16; i++) {
		playerGraphHandles[i] = -1;
	}
	animCounter = 0;
	facingRight = true;
	caveTimer = 0;
	isHidden = false;
	playerX = 0.0f;
	playerY = 0.0f;
	playerSpeed = 5.0f;
	targetX = 0.0f;
	targetY = 0.0f;
	isMoving = false;
	lastSignal = "None";
	mEnterTimer = 0;
	mIsEnteringStage = false;
	mIsReturningTitle = false;
	mReturnTimer = 0;
}

// デストラクタ
StageSelectScene::~StageSelectScene()
{
}

// 初期化処理：リソースの読み込みとプレイヤー・フラグの初期位置設定を行う
void StageSelectScene::Initialize()
{
	mapGraphHandle = LoadGraph(BackGroundGraphPath::SelectStageMap.c_str());
	LoadDivGraph(
		CharacterGraphPath::PlayerAnimation.c_str(),
		16,
		4,
		4,
		128,
		128,
		playerGraphHandles
	);

	playerX = PlayerInitX;
	playerY = PlayerY;
	targetX = Stage1_X;
	targetY = PlayerY;
	isMoving = true;

	isMoving = true;
	lastSignal = "None";
	facingRight = true;
	mIsEnteringStage = false;
	mEnterTimer = 0;
	mIsReturningTitle = false;
	mReturnTimer = 0;
	animCounter = 0;
	caveTimer = 0;
	isHidden = false;

	Master::gPlayerLive = PlayerConstants::MaxLive;

	// 現在のプレイ時の死亡回数だけリセットする
	for (int i = 0; i < Master::MaxStageCount; ++i)
	{
		Master::gDeathCounts[i] = 0;
	}

	mFade.StartFadeIn(30);
}

// 移動処理：プレイヤーの移動アニメーションや自動ルート分岐、キー入力による移動先の設定を行う
void StageSelectScene::Move()
{
	// 待機モーションも動かしたいからifの外に移動した
	animCounter++;

	if (isMoving)
	{
		float dx = targetX - playerX;
		float dy = targetY - playerY;
		float dist = sqrtf(dx * dx + dy * dy);

		// 移動方向に応じてスプライトの向きを切り替える
		if (dx > 0.0f) {
			facingRight = true;
		}
		else if (dx < 0.0f) {
			facingRight = false;
		}

		// 目的地に到達した際の処理
		if (dist <= playerSpeed)
		{
			playerX = targetX;
			playerY = targetY;
			isMoving = false;
			animCounter = 0;
		}
		else
		{
			// 等速で目的地に向かって座標を更新する
			playerX += (dx / dist) * playerSpeed;
			playerY += (dy / dist) * playerSpeed;
		}
	}
	else
	{
		if (CheckHitKey(KEY_INPUT_A) == 1)
		{
			if (playerX == Stage3_X && playerY == PlayerY)
			{
				lastSignal = "LEFT";
				targetX = Stage2_X;
				targetY = PlayerY;
				isMoving = true;
			}
			else if (playerX == Stage2_X && playerY == PlayerY)
			{
				lastSignal = "LEFT";
				targetX = Stage1_X;
				targetY = PlayerY;
				isMoving = true;
			}
		}
		else if (CheckHitKey(KEY_INPUT_D) == 1)
		{
			if (playerX == Stage1_X && playerY == PlayerY)
			{
				lastSignal = "RIGHT";
				targetX = Stage2_X;
				targetY = PlayerY;
				isMoving = true;
			}
			else if (playerX == Stage2_X && playerY == PlayerY)
			{
				lastSignal = "RIGHT";
				targetX = Stage3_X;
				targetY = PlayerY;
				isMoving = true;
			}
		}
	}
}

// 更新処理：ステージ移行フェード、タイトル帰還フェード、洞窟内のタイマー処理、キー入力を管理する
void StageSelectScene::Update()
{
	mFade.Update();

	// デバッグで解放している
	if (InputManager::CheckDownKey(KEY_INPUT_8))
	{
		Master::gStageCleared[0] = true;
		Master::gStageCleared[1] = true;
		Master::gStageCleared[2] = true;
	}


	// ステージ突入時の演出タイマー処理
	if (mIsEnteringStage) {
		mEnterTimer++;
		if (mFade.IsFadeOutFinished()) {
			if (Master::gDeathCounts[Master::gCurrentStageIndex] == -1)
			{


				Master::gTotalDeathCounts[Master::gCurrentStageIndex] += Master::gDeathCounts[Master::gCurrentStageIndex];
				Master::gDeathCounts[Master::gCurrentStageIndex] = 0;
			}
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_GAME);
		}
		return;
	}
	// タイトル帰還時の演出タイマー処理
	if (mIsReturningTitle) {
		mReturnTimer++;
		if (mFade.IsFadeOutFinished()) {
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TITLE);
		}
		return;
	}
	

	// プレイヤーの移動処理を呼び出し
	Move();

	// Qキーでタイトルに戻るフラグを立てる
	if (InputManager::CheckDownKey(KEY_INPUT_Q) && !mIsEnteringStage && !mIsReturningTitle && !isHidden && !isMoving)
	{
		mIsReturningTitle = true;
		mReturnTimer = 0;
		mFade.StartFadeOut(60);
	}

	// Enterキーで現在の位置のステージへ入る処理
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN) && !mIsEnteringStage && !isHidden && !isMoving)
	{
		Master::mpGameManager->GetSoundManager()->PlaySE(SoundManager::SE_DECIDE, SoundSetting::DecideVolume);

		int nodeIndex = -1;
		if (playerX == Stage1_X && playerY == PlayerY) nodeIndex = 0;
		else if (playerX == Stage2_X && playerY == PlayerY) nodeIndex = 1;
		else if (playerX == Stage3_X && playerY == PlayerY) nodeIndex = 2;

		// Enter処理
		bool locked = false;
		if (nodeIndex == 0) locked = false;                          // 1-1 は常に解放
		else if (nodeIndex == 1) locked = !Master::gStageCleared[0]; // 1-2 は 1-1 クリアで解放
		else if (nodeIndex == 2) locked = !Master::gStageCleared[1]; // 1-3 は 1-2 クリアで解放


		// ロックされていなければステージへ進む
		if (!locked) {
			Master::gCurrentStageIndex = nodeIndex;
			mIsEnteringStage = true;
			mEnterTimer = 0;
			mFade.StartFadeOut(60);
		}
	}

	Scene::Update();
}



// 描画処理：背景マップ、プレイヤーキャラクター、UI情報を描画する
void StageSelectScene::Draw()
{
	// 背景マップの描画
	if (mapGraphHandle != -1)
	{
		DrawExtendGraph(0, 0, Utility::SCREEN_WIDTH, Utility::SCREEN_HEIGHT, mapGraphHandle, TRUE);
	}

	// プレイヤーキャラの描画
	if (!isHidden && playerGraphHandles[0] != -1)
	{
		int baseFrame = isMoving ? 4 : 0;
		int currentFrame = baseFrame + ((animCounter / 10) % 4);

		// Enter押下後の演出進行度（0.0 → 1.0）
		float enterT = 0.0f;
		if (mIsEnteringStage)
		{
			enterT = static_cast<float>(mEnterTimer) / static_cast<float>(kEnterAnimFrames);
			if (enterT > 1.0f) enterT = 1.0f;
		}
		// 最初は速く、後半はゆっくり動くイーズアウト
		const float easeT = 1.0f - (1.0f - enterT) * (1.0f - enterT);

		const float drawScale = 1.0f - (1.0f - kEnterMinScale) * easeT;
		const float drawY = playerY - kEnterRiseY * easeT;

		// 中心基準で描画するため、playerX / drawY をそのまま中心座標として使う
		DrawRectRotaGraph(
			static_cast<int>(playerX),
			static_cast<int>(drawY),
			1, 0,
			PlayerAnimState::FRAME_WIDTH,
			PlayerAnimState::FRAME_HEIGHT,
			drawScale,
			0.0,
			playerGraphHandles[currentFrame],
			TRUE,
			!facingRight
		);
	}



	// 停止中で非表示でなければ、ステージ詳細情報を描画する
	if (!isMoving && !isHidden)
	{
		DrawStageString();
		TitleDrawString();
	}

	Scene::Draw();
	mFade.Draw();
}

void StageSelectScene::TitleDrawString()
{
	// タイトルに戻る操作説明の描画
	unsigned int returnColor = GetColor(255, 255, 255);
	if (mIsReturningTitle) {
		returnColor = (mReturnTimer % 10 < 5) ? GetColor(255, 255, 0) : GetColor(150, 150, 0);
	}

	// 背景のBoxの描画クラスの呼び出し
	mBackgroundBox.DrawBackgroundBox(
		TitleBoxPos,
		TitleBoxPos,
		TitleBoxWidth,
		TitleBoxHeight,
		GetColor(0, 0, 0),
		GetColor(255, 255, 255),
		200,
		1
	);
	Master::mpGameManager->GetFontManager()->DrawDotString(
		TitleStringX, TitleStringY, TitleStringSize, returnColor, "Q: タイトルに戻る");
}

// ステージ名や説明文、死亡回数などを記載したダイアログUIを描画する
void StageSelectScene::DrawStageString()
{
	const char* stageName = nullptr;
	const char* stageDesc = nullptr;
	bool locked = false;
	int nodeIndex = -1;

	// プレイヤーの座標に応じてどのステージにいるかを判定する
	if (playerX == Stage1_X && playerY == PlayerY) {
		nodeIndex = 0;
		stageName = "Stage 1-1 始まりの森林";
		stageDesc = "「まずはチュートリアルをプレイしてね！」";
	}
	else if (playerX == Stage2_X && playerY == PlayerY) {
		nodeIndex = 1;
		stageName = "Stage 1-2 マウンテン山々山";
		locked = !Master::gStageCleared[0];
		stageDesc = locked ? "【解放条件】 Stage 1-1をクリアしよう" : "「基本操作を活かして進んでみよう！」";
	}
	else if (playerX == Stage3_X && playerY == PlayerY) {
		nodeIndex = 2;
		stageName = "Stage 1-3 魔王の城 (BOSS)";
		locked = !Master::gStageCleared[1];
		stageDesc = locked ? "【解放条件】 Stage 1-2をクリアしよう" : "「ついにボス戦！ 準備はいいか？」";
	}

	if (stageName)
	{
		int px = (int)playerX;
		int py = (int)playerY;

		int boxW = 660;
		int boxH = 180;
		int boxX = px - boxW / 2;
		int boxY = py - 500;

		// 画面外にはみ出さないように描画位置を調整する
		if (boxX < 30) boxX = 30;
		if (boxX + boxW > Utility::SCREEN_WIDTH - 30) boxX = Utility::SCREEN_WIDTH - 30 - boxW;
		if (boxY < 20) boxY = py + 100;

		// 背景のBox
		mBackgroundBox.DrawBackgroundBox(
			boxX,
			boxY,
			boxX + boxW,
			boxY + boxH,
			GetColor(0, 0, 0),
			GetColor(255, 255, 255),
			200,
			1
		);

		// ロック状態に応じて枠線の色を変化させる（ロック中：赤、アンロック：灰色）
		unsigned int borderColor = locked ? GetColor(255, 50, 50) : GetColor(200, 200, 200);
		DrawBox(boxX + 3, boxY + 3, boxX + boxW - 3, boxY + boxH - 3, borderColor, FALSE);

		FontManager* fontMgr = Master::mpGameManager->GetFontManager();

		// 中央揃え用のヘルパー：ダイアログ中央に、指定サイズの文字列を描画する
		auto drawCentered = [&](int y, int fontSize, unsigned int color, const char* text)
			{
				int textW = fontMgr->GetDotStringWidth(fontSize, text);
				int x = boxX + (boxW - textW) / 2;
				fontMgr->DrawDotString(x, y, fontSize, color, text);
			};

		// ステージタイトル（中央揃え）
		drawCentered(boxY + 15, 36, GetColor(255, 255, 0), stageName);

		// 死亡回数（中央揃え）
		char deathText[128];
		sprintf_s(deathText, "今回の死亡回数: %d 回", Master::gDeathCounts[nodeIndex]);
		drawCentered(boxY + 60, 24, GetColor(255, 100, 100), deathText);

		// ステージ説明文（中央揃え）
		unsigned int descColor = locked ? GetColor(255, 100, 100) : GetColor(255, 255, 255);
		drawCentered(boxY + 95, 32, descColor, stageDesc);

		// エンターキーを促すプロンプト、またはロック状態の文字列（中央揃え）
		if (!locked) {
			unsigned int enterColor = GetColor(200, 200, 255);
			if (mIsEnteringStage) {
				enterColor = (mEnterTimer % 10 < 5) ? GetColor(255, 255, 0) : GetColor(150, 150, 0);
			}
			drawCentered(boxY + 125, 24, enterColor, "Press ENTER to Start");
		}
		else {
			drawCentered(boxY + 125, 24, GetColor(100, 100, 100), "LOCKED");
		}
	}
}

// 終了処理：読み込んだグラフィックリソースの解放を行う
void StageSelectScene::Finalize()
{
	for (int i = 0; i < 16; i++)
	{
		if (playerGraphHandles[i] != -1)
		{
			DeleteGraph(playerGraphHandles[i]);
			playerGraphHandles[i] = -1;
		}
	}

	if (mapGraphHandle != -1)
	{
		DeleteGraph(mapGraphHandle);
		mapGraphHandle = -1;
	}
}