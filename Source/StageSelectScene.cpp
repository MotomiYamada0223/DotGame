#include "StageSelectScene.h"
#include "DxLib.h"
#include "Utility.h"
#include "InputManager.h"
#include "Master.h"
#include "SceneManager.h"
#include <math.h> 
#include "GameConstants.h"
#include "PlayerAnimState.h"

// コンストラクタ：各種変数の初期化を行う
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
	LoadDivGraph(CharacterGraphPath::PlayerAnimation.c_str(),
		16, 4, 4, 128, 128, playerGraphHandles);

	playerX = 236.0f;
	playerY = -100.0f;
	targetX = 236.0f;
	targetY = 232.0f;
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

			// 特定の座標に到達した際、自動で次の移動先へつなげる自動ルーティング処理
			if (playerX == 1250.0f && playerY == 909.0f)
			{
				if (lastSignal == "DOWN")
				{
					targetX = 1590.0f;
					targetY = 909.0f;
					isMoving = true;
					lastSignal = "RIGHT";
				}
				else if (lastSignal == "LEFT")
				{
					targetX = 1250.0f;
					targetY = 806.0f;
					isMoving = true;
					lastSignal = "UP";
				}
			}
			else if (playerX == 1590.0f && playerY == 909.0f)
			{
				if (lastSignal == "RIGHT")
				{
					targetX = 1590.0f;
					targetY = 551.0f;
					isMoving = true;
					lastSignal = "UP";
				}
				else if (lastSignal == "DOWN")
				{
					targetX = 1250.0f;
					targetY = 909.0f;
					isMoving = true;
					lastSignal = "LEFT";
				}
			}
			else if (playerX == 676.0f && playerY == 525.0f) // 上側洞窟の入り口
			{
				if (lastSignal == "UP")
				{
					isHidden = true;
					caveTimer = 180;
					isMoving = false;
					lastSignal = "UP_FROM_LEFT";
				}
			}
			else if (playerX == 1250.0f && playerY == 806.0f) // 右側洞窟の出口
			{
				if (lastSignal == "UP")
				{
					isHidden = true;
					caveTimer = 180;
					isMoving = false;
					lastSignal = "UP_FROM_RIGHT";
				}
			}

			if (!isMoving && caveTimer == 0) {
				lastSignal = "None";
			}
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
		// 停止中のキー入力による移動先ノードの決定
		if (CheckHitKey(KEY_INPUT_W) == 1)
		{
			if (playerX == 236.0f && playerY == 909.0f)
			{
				lastSignal = "UP";
				targetX = 236.0f;
				targetY = 232.0f;
				isMoving = true;
			}
			else if (playerX == 676.0f && playerY == 909.0f) // ノード2から上側洞窟へ
			{
				lastSignal = "UP";
				targetX = 676.0f;
				targetY = 525.0f;
				isMoving = true;
			}
		}
		else if (CheckHitKey(KEY_INPUT_S) == 1)
		{
			if (playerX == 236.0f && playerY == 232.0f)
			{
				lastSignal = "DOWN";
				targetX = 236.0f;
				targetY = 909.0f;
				isMoving = true;
			}
			else if (playerX == 1590.0f && playerY == 551.0f) // お城から下方へ
			{
				lastSignal = "DOWN";
				targetX = 1590.0f;
				targetY = 909.0f;
				isMoving = true;
			}
		}
		else if (CheckHitKey(KEY_INPUT_A) == 1)
		{
			if (playerX == 676.0f && playerY == 909.0f)
			{
				lastSignal = "LEFT";
				targetX = 236.0f;
				targetY = 909.0f;
				isMoving = true;
			}
		}
		else if (CheckHitKey(KEY_INPUT_D) == 1)
		{
			if (playerX == 236.0f && playerY == 909.0f)
			{
				lastSignal = "RIGHT";
				targetX = 676.0f;
				targetY = 909.0f;
				isMoving = true;
			}
		}
	}
}

// 更新処理：ステージ移行フェード、タイトル帰還フェード、洞窟内のタイマー処理、キー入力を管理する
void StageSelectScene::Update()
{
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
		if (mEnterTimer > 60) {
			if (Master::gDeathCounts[Master::gCurrentStageIndex] == -1)
			{
				// 累計にするためにコメントアウト
				//Master::gDeathCounts[Master::gCurrentStageIndex] = 0;
			}
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_GAME);
		}
		return;
	}
	// タイトル帰還時の演出タイマー処理
	if (mIsReturningTitle) {
		mReturnTimer++;
		if (mReturnTimer > 60) {
			Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_TITLE);
		}
		return;
	}
	// 洞窟内でのワープ（移動演出）中のタイマー処理
	if (caveTimer > 0)
	{
		caveTimer--;
		if (caveTimer == 0)
		{
			isHidden = false;
			if (lastSignal == "UP_FROM_LEFT")
			{
				playerX = 1250.0f;
				playerY = 806.0f;
				targetX = 1250.0f;
				targetY = 909.0f;
				isMoving = true;
				lastSignal = "DOWN";
			}
			else if (lastSignal == "UP_FROM_RIGHT")
			{
				playerX = 676.0f;
				playerY = 525.0f;
				targetX = 676.0f;
				targetY = 909.0f;
				isMoving = true;
				lastSignal = "DOWN";
			}
		}
		Scene::Update();
		return;
	}

	// プレイヤーの移動処理を呼び出し
	Move();

	// Qキーでタイトルに戻るフラグを立てる
	if (InputManager::CheckDownKey(KEY_INPUT_Q) && !mIsEnteringStage && !mIsReturningTitle && !isHidden && !isMoving)
	{
		mIsReturningTitle = true;
		mReturnTimer = 0;
	}

	// Enterキーで現在の位置のステージへ入る処理
	if (InputManager::CheckDownKey(KEY_INPUT_RETURN) && !mIsEnteringStage && !isHidden && !isMoving)
	{
		int nodeIndex = 0;
		if (playerX == 236.0f && playerY == 909.0f) nodeIndex = 1;
		else if (playerX == 676.0f && playerY == 909.0f) nodeIndex = 2;
		else if (playerX == 1590.0f && playerY == 551.0f) nodeIndex = 3;

		bool locked = false;
		if (nodeIndex == 1) locked = !Master::gStageCleared[0];
		else if (nodeIndex == 2) locked = !Master::gStageCleared[1];
		else if (nodeIndex == 3) locked = !Master::gStageCleared[2];

		// ロックされていなければステージへ進む
		if (!locked) {
			Master::gCurrentStageIndex = nodeIndex;
			mIsEnteringStage = true;
			mEnterTimer = 0;
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

		// 指定の場所だけ描画する   
		// 親クラスの共通関数を使用してワールド座標からスクリーン座標へ変換する   
		const int screenX = static_cast<int>((playerX) - PlayerAnimState::FRAME_WIDTH / 2);
		const int screenY = static_cast<int>((playerY) - PlayerAnimState::FRAME_HEIGHT / 2);


		DrawRectGraph(screenX, screenY, 1, 0,
			PlayerAnimState::FRAME_WIDTH,
			PlayerAnimState::FRAME_HEIGHT,
			playerGraphHandles[currentFrame],
			true,
			!facingRight
		);
	}

	// 停止中で非表示でなければ、ステージ詳細情報を描画する
	if (!isMoving && !isHidden)
	{
		DrawStageString();
		TitleDrawString();
	}

	// シーン遷移時のフェードアウト用黒画面の描画
	if (mIsEnteringStage || mIsReturningTitle) {
		int timer = mIsEnteringStage ? mEnterTimer : mReturnTimer;
		int fadeAlpha = (timer * 255) / 60;
		if (fadeAlpha > 255) fadeAlpha = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
		DrawBox(0, 0, Utility::SCREEN_WIDTH, Utility::SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	Scene::Draw();
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
	int nodeIndex = 0;

	
	// プレイヤーの座標に応じてどのステージにいるかを判定する
	if (playerX == 236.0f && playerY == 232.0f) {
		nodeIndex = 0;
		stageName = "Stage 1-1 始まりの森林";
		stageDesc = "「まずはチュートリアルをプレイしてね！」";
	}
	else if (playerX == 236.0f && playerY == 909.0f) {
		nodeIndex = 1;
		stageName = "Stage 1-2 緑の平原";
		locked = !Master::gStageCleared[0];
		stageDesc = locked ? "【解放条件】 Stage 1-1をクリアしよう" : "「基本操作を活かして進んでみよう！」";
	}
	else if (playerX == 676.0f && playerY == 909.0f) {
		nodeIndex = 2;
		stageName = "Stage 1-3 洞窟前の試練";
		locked = !Master::gStageCleared[1];
		stageDesc = locked ? "【解放条件】 Stage 1-2をクリアしよう" : "「洞窟を抜ければ、ボスの城はすぐそこだ！」";
	}
	else if (playerX == 1590.0f && playerY == 551.0f) {
		nodeIndex = 3;
		stageName = "Stage 1-4 魔王の城 (BOSS)";
		locked = !Master::gStageCleared[2];
		stageDesc = locked ? "【解放条件】 Stage 1-3をクリアしよう" : "「ついにボス戦！ 準備はいいか？」";
	}

	if (stageName)
	{
		int px = (int)playerX;
		int py = (int)playerY;

		int boxW = 860;
		int boxH = 150; // 3行分のテキストが収まるように高さを拡張
		int boxX = px - boxW / 2;
		int boxY = py - 300;

		// 画面外にはみ出さないように描画位置を調整する
		if (boxX < 30) boxX = 30;
		if (boxX + boxW > Utility::SCREEN_WIDTH - 30) boxX = Utility::SCREEN_WIDTH - 30 - boxW;
		if (boxY < 20) boxY = py + 100;


		// 背景のBoxの描画クラスの呼び出し
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




		// ステージタイトルの描画
		Master::mpGameManager->GetFontManager()->DrawDotString(boxX + 25, boxY + 15, 36, GetColor(255, 255, 0), stageName);

		// 死亡回数の描画
		char deathText[64];
		if (Master::gDeathCounts[nodeIndex] == -1) {
			sprintf_s(deathText, "死亡回数: - 回");
		}
		else {
			sprintf_s(deathText, "死亡回数: %d 回", Master::gDeathCounts[nodeIndex]);
		}
		Master::mpGameManager->GetFontManager()->DrawDotString(boxX + 25, boxY + 60, 24, GetColor(255, 100, 100), deathText);

		// ステージ説明文の描画
		unsigned int descColor = locked ? GetColor(255, 100, 100) : GetColor(255, 255, 255);
		Master::mpGameManager->GetFontManager()->DrawDotString(boxX + 25, boxY + 95, 32, descColor, stageDesc);

		// エンターキーを促すプロンプト、またはロック状態の文字列を描画
		if (!locked) {
			unsigned int enterColor = GetColor(200, 200, 255);
			if (mIsEnteringStage) {
				enterColor = (mEnterTimer % 10 < 5) ? GetColor(255, 255, 0) : GetColor(150, 150, 0);
			}
			Master::mpGameManager->GetFontManager()->DrawDotString(boxX + boxW - 300, boxY + 115, 24, enterColor, "Press ENTER to Start");
		}
		else {
			Master::mpGameManager->GetFontManager()->DrawDotString(boxX + boxW - 300, boxY + 115, 24, GetColor(100, 100, 100), "LOCKED");
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