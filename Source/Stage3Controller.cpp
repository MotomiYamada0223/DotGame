#include "Stage3Controller.h"
#include "Stage3BossDragon.h"
#include "BlockMap.h"
#include "Player.h"
#include "GameConstants.h"
#include "Master.h"

Stage3Controller::Stage3Controller()
	: mSkyBackgroundHandle(-1)
	, mpBossDragon(nullptr)
	, mbMovieStarted(false)
{
}

Stage3Controller::~Stage3Controller()
{
	Finalize();
}

void Stage3Controller::Initialize(BlockMap& blockMap)
{
	Finalize();

	// ステージ3用の奥背景読み込み（洞窟ステージと同じ背景画像を使用）
	mSkyBackgroundHandle = LoadGraph(ScrollGraphPath::StageCave);

	// ステージ3のマップ画像と当たり判定画像を読み込み
	blockMap.Load(
		BlockMapGraphPath::Stage3Background,
		BlockMapGraphPath::Stage3Collision
	);
	blockMap.ResetScroll();
	blockMap.SetVerticalScrollLocked(false);
	blockMap.SetAllowRightEdgeFall(false);

	// ステージ3のギミック・マグマ・ムービー・ボス戦引きカメラを初期化
	mGimmick.Initialize(blockMap);
	mMagma.Initialize();
	mBossMovie.Initialize();
	mBossCamera.Initialize();
	mEffectManager.Initialize();
	mbMovieStarted = false;
	mpBossDragon   = nullptr;
}

VECTOR Stage3Controller::GetInitialSpawnPosition(const BlockMap& blockMap) const
{
	// 【Stage3.png 5724x1692拡大版 対応】左側通路（天井Y=940, 床Y=1262）にスポーンし、床の高さにスナップ
	float spawnX = 400.0f;
	float spawnY = 1080.0f;

	while (blockMap.GetCollisionType(static_cast<int>(spawnX), static_cast<int>(spawnY) + 50) == BlockMap::CollisionType::None && spawnY < 1650.0f)
	{
		spawnY += 1.0f;
	}
	spawnY -= 1.0f;

	return VGet(spawnX, spawnY, 0.0f);
}

void Stage3Controller::SpawnBossOnFarLedge(BlockMap& blockMap)
{
	(void)blockMap;
	// ボス部屋の一番奥の足場（X = 5330, Y = 1189）にドラゴンを配置
	float spawnX = Stage3BossDragon::SPAWN_X;
	float spawnY = Stage3BossDragon::SPAWN_Y;
	mpBossDragon = new Stage3BossDragon(VGet(spawnX, spawnY, 0.0f));
	mpBossDragon->SetCutsceneMode(true);
}

void Stage3Controller::SetupInitialCamera(BlockMap& blockMap, Player* player)
{
	if (player != nullptr)
	{
		player->SetSpawnPosition(player->GetPosition());
		int initScrollY = static_cast<int>(player->GetPosition().y) - 720;
		if (initScrollY < 0) initScrollY = 0;
		int maxScrollY = blockMap.GetCurrentHeight() - ScreenSize::ScrrenHeight;
		if (maxScrollY < 0) maxScrollY = 0;
		if (initScrollY > maxScrollY) initScrollY = maxScrollY;
		blockMap.SetScrollY(initScrollY);
	}

	// 奥の足場にドラゴンを配置しておく
	SpawnBossOnFarLedge(blockMap);
}

void Stage3Controller::UpdateBeforePlayerMove(Player* player)
{
	mGimmick.UpdatePlatformsBeforePlayerMove(player);
}

void Stage3Controller::Update(BlockMap& blockMap, Player* player)
{
	// 1. 壁が降りてくる処理を開始したら、即座にプレイヤーの動きを一時停止してムービー待機にする
	if (mGimmick.IsGateTriggered() && !mbMovieStarted && player != nullptr && !player->GetIsDead())
	{
		player->SetInputDisabled(true);
	}

	// 2. ギミックとマグマの更新
	mGimmick.Update(blockMap, player);
	mMagma.Update(blockMap, player);

	// 3. 壁が完全に降りきったら、奥の足場のドラゴンへカメラを寄せて映してから元のプレイヤー視点へ戻すムービーを開始！
	if (mGimmick.IsGateClosed() && !mbMovieStarted)
	{
		mbMovieStarted = true;
		mBossMovie.StartMovie(blockMap, player, mpBossDragon);
		if (Master::mpGameManager && Master::mpGameManager->GetSoundManager())
		{
			Master::mpGameManager->GetSoundManager()->PlayBGM(SoundManager::BGM_STAGE3_BOSS, true);
		}
	}

	// 3-2. 第1形態撃破時、第2形態「アルティメットドラゴン」黄金覚醒ムービーを開始！
	if (mpBossDragon != nullptr && mpBossDragon->ConsumeAwakeningMovieRequest())
	{
		mGimmick.RestoreAllPlatforms();
		mBossMovie.StartAwakeningMovie(blockMap, player, mpBossDragon);
	}

	// 4. ムービーカメラの更新（PlayerMoveによる通常のカメラスクロールを上書きしてドラゴンへパン＆復帰）
	if (mBossMovie.IsPlaying())
	{
		mBossMovie.Update(blockMap, player, mpBossDragon);
	}

	// 5. ボス戦中（ムービー完了後）は引きカメラを有効にしてボス部屋全体を映す
	bool isBossFightActive = (mbMovieStarted && !mBossMovie.IsPlaying());
	mBossCamera.Update(blockMap, player, mpBossDragon, isBossFightActive);
	if (mBossCamera.IsActive())
	{
		blockMap.LockScroll(true);
	}

	// 6. ステージ3専用2D Effekseerエフェクトの更新
	int viewW = mBossCamera.IsActive() ? mBossCamera.GetCurrentViewWidth()  : ScreenSize::ScrrenWidth;
	int viewH = mBossCamera.IsActive() ? mBossCamera.GetCurrentViewHeight() : ScreenSize::ScrrenHeight;
	mEffectManager.Update(blockMap, viewW, viewH);
}

void Stage3Controller::BeginBossCameraDraw()
{
	mBossCamera.BeginWorldDraw();
}

void Stage3Controller::EndBossCameraDraw()
{
	mBossCamera.EndWorldDraw();
}

void Stage3Controller::DrawBackground(const BlockMap& blockMap)
{
	(void)blockMap;
	int viewW = ScreenSize::ScrrenWidth;
	int viewH = ScreenSize::ScrrenHeight;
	int curScreen = GetDrawScreen();
	if (curScreen > 0)
	{
		GetGraphSize(curScreen, &viewW, &viewH);
	}

	DrawBox(0, 0, viewW, viewH, GetColor(18, 20, 28), TRUE);
	if (mSkyBackgroundHandle != -1)
	{
		DrawExtendGraph(0, 0, viewW, viewH, mSkyBackgroundHandle, FALSE);
	}
}

void Stage3Controller::DrawForeground(const BlockMap& blockMap, Player* player)
{
	// 台の下のくぼみにマグマを描画し、その手前に浮遊台・閉じ込め壁を描画
	mMagma.Draw(blockMap);
	mGimmick.Draw(blockMap, player);
}

void Stage3Controller::DrawEffects(const BlockMap& blockMap)
{
	(void)blockMap;
	mEffectManager.Draw();
}

void Stage3Controller::DrawUI(BlockMap& blockMap, Player* player)
{
	// ムービー再生中は上下のシネマ黒帯とボス名テロップを描画
	if (mBossMovie.IsPlaying())
	{
		mBossMovie.Draw(blockMap, mpBossDragon);
	}
	else if (mbMovieStarted && mpBossDragon != nullptr)
	{
		// ムービー終了後の戦闘中は画面下部にドラゴンのHPバーを描画
		mpBossDragon->DrawBossUI(player, blockMap);
	}
}

void Stage3Controller::ResetOnRespawn(BlockMap& blockMap, Player* player)
{
	blockMap.Load(
		BlockMapGraphPath::Stage3Background,
		BlockMapGraphPath::Stage3Collision
	);
	blockMap.ResetScroll();
	blockMap.SetVerticalScrollLocked(false);
	blockMap.SetAllowRightEdgeFall(false);
	blockMap.mShakeX = 0;
	blockMap.mShakeY = 0;

	// ステージ3ギミック・マグマ・ムービー・ボス戦引きカメラをリセット
	mGimmick.Reset(blockMap);
	mMagma.Reset();
	mBossMovie.Reset();
	mBossCamera.Reset();
	mEffectManager.StopAll();
	mbMovieStarted = false;
	mpBossDragon   = nullptr;

	// 奥の足場にドラゴンを再配置
	SpawnBossOnFarLedge(blockMap);

	if (player != nullptr)
	{
		player->SetWindFloatMode(false);
		player->SetIsGrounded(true);
		player->SetVelocityY(0.0f);
		player->SetInputDisabled(false);
		int initScrollY = static_cast<int>(player->GetPosition().y) - 720;
		if (initScrollY < 0) initScrollY = 0;
		int maxScrollY = blockMap.GetCurrentHeight() - ScreenSize::ScrrenHeight;
		if (maxScrollY < 0) maxScrollY = 0;
		if (initScrollY > maxScrollY) initScrollY = maxScrollY;
		blockMap.SetScrollY(initScrollY);
	}
}

void Stage3Controller::Finalize()
{
	mGimmick.Finalize();
	mMagma.Finalize();
	mBossMovie.Reset();
	mBossCamera.Finalize();
	mEffectManager.Finalize();
	mpBossDragon   = nullptr;
	mbMovieStarted = false;
	if (mSkyBackgroundHandle != -1)
	{
		DeleteGraph(mSkyBackgroundHandle);
		mSkyBackgroundHandle = -1;
	}
}
