#include "LeafCutter.h"
#include "GameConstants.h"
#include "DeathReason.h"
#include "InputManager.h"
#include "Master.h"

namespace
{
	// 葉っぱの挙動に関する定数
	constexpr int   kSpawnWidthMargin = 800;       // 画面幅に対して左右に広げるスポーン範囲のマージン（画面外から自然に降らせるため）
	constexpr int   kSpawnOffsetLeft = 400;        // スポーン開始位置を画面左側にオフセットする幅（中央基準にするため）
	constexpr float kSpawnTopOffset = 50.0f;       // 画面上部から少し上にスポーンさせるためのオフセット
	constexpr float kMaxSpawnLimitX = 7800.0f;     // ステージの進行度に応じて葉っぱの生成を停止するX座標の閾値

	// 物理挙動・ランダム値の係数
	constexpr float kFallSpeedBase = 0.5f;         // 葉っぱの基本落下速度
	constexpr float kFallSpeedRandom = 150.0f;     // 落下速度のランダム幅を決める係数
	constexpr float kSwaySpeedBase = 0.02f;        // 左右の揺れの基本速度
	constexpr float kSwaySpeedRandom = 40.0f;      // 揺れ速度のランダム幅を決める係数
	constexpr float kSwayAmpBase = 3.0f;           // 左右の揺れの基本幅（振幅）
	constexpr float kSwayAmpRandom = 300.0f;       // 揺れの幅のランダム幅を決める係数
	constexpr float kPhaseScale = 100.0f;          // 乱数からラジアンやスケールを算出するためのスケーリング用 divisor
	constexpr float kRotationSpeedBase = 50.0f;    // 回転速度の基準値（プラスマイナスの振れ幅を作るため）
	constexpr float kRotationSpeedScale = 800.0f;  // 回転速度を滑らかにするためのスケーリング用 divisor
	constexpr float kScaleBase = 0.2f;             // 葉っぱの基本拡大率（最小サイズ）
	constexpr float kScaleRandom = 20.0f;          // 拡大率のランダム幅を決める係数
	constexpr int   kShadowProbability = 50;       // 葉っぱが影（背景演出）になる確率（50%）
	constexpr float kWindScaleFactor = 0.01f;      // スケール（大きさ）に応じて風の影響を受けやすくするための係数
	constexpr float kSwoopPhaseFactor = 0.7f;      // 縦方向のふらつき（周期）を変化させるための位相係数
	constexpr float kSwoopAmpFactor = 0.6f;        // 縦方向のふらつきの強さを調整する係数

	// 画面外判定のマージン
	constexpr int   kDeleteMarginBottom = 200;     // 画面下部に落ちた際、完全に消去するまでの余白ピクセル数
	constexpr int   kDeleteMarginSide = 400;       // 画面の左右からはみ出た際、消去するまでの余白ピクセル数
	constexpr int   kDeleteMarginTop = 300;        // 画面上部に外れた際、消去するまでの余白ピクセル数

	// 当たり判定サイズ
	constexpr float kPlayerHalfWidth = 16.0f;      // プレイヤーの当たり判定の幅の半分
	constexpr float kPlayerHalfHeight = 50.0f;     // プレイヤーの当たり判定の高さの半分
	constexpr float kLeafHalfSize = 12.0f;         // 葉っぱの当たり判定の矩形サイズ（中心からの半径相当）

	// 葉っぱ生成確率
	constexpr int LeafSpawn = 2; // 葉っぱの生成確率今は2%
}

LeafCutter::LeafCutter()
	: mnLeafHandle(-1)
	, mnLeafShadowHandle(-1)
	, mLeaves()
	, mbStopSpawning(false)
{
	// 描画に使用する通常葉っぱと影の画像を読み込む
	mnLeafHandle = LoadGraph("Resource/Leaf/leaf.png");
	mnLeafShadowHandle = LoadGraph("Resource/Leaf/LeafShadow.png");

}

// 読み込んだグラフィックのリソースを解放する
LeafCutter::~LeafCutter()
{
	if (mnLeafHandle != -1) DeleteGraph(mnLeafHandle);
	if (mnLeafShadowHandle != -1) DeleteGraph(mnLeafShadowHandle);

}

// 葉っぱの管理リストをリセットする
void LeafCutter::LeafReset()
{
	mLeaves.clear();
}

// 葉っぱの描画処理を行う
void LeafCutter::LeafDraw(BlockMap& block)
{
	if (Master::gCurrentStageIndex != 0)
	{
		return;
	}

	for (const auto& leaf : mLeaves)
	{
		// スクロール量を考慮して描画座標を算出する
		int drawX = static_cast<int>(leaf.x) - block.GetScrollX();
		int drawY = static_cast<int>(leaf.y) - block.GetScrollY();

		// 影フラグに応じて描画するグラフィックを切り替える
		int handle = leaf.isShadow ? mnLeafShadowHandle : mnLeafHandle;

		if (handle != -1)
		{
			// 影の場合は半透明で描画する
			if (leaf.isShadow)
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

			// 拡大率や回転を反映して葉っぱを描画する
			DrawRotaGraph(
				drawX,
				drawY,
				leaf.scale,
				leaf.rotation,
				handle,
				TRUE
			);

			// 描画モードを通常に戻す
			if (leaf.isShadow)
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		int debagDraw = static_cast<int>(kLeafHalfSize);
		// 通常の葉っぱだけ当たり判定を赤枠で表示
		if (!leaf.isShadow)
		{
			int left = drawX - debagDraw;
			int right = drawX + debagDraw;
			int top = drawY - debagDraw;
			int bottom = drawY + debagDraw;

			DrawBox(
				left, top, right, bottom,
				GetColor(255, 0, 0),
				FALSE
			);
		}
	}
}

void LeafCutter::LeafDebug()
{
	if (mbStopSpawning)
	{
		LeafReset();
	}

	// デバッグ用
	if (InputManager::CheckDownKey(KEY_INPUT_3))
	{
		mbStopSpawning = !mbStopSpawning;
	}
}

// 葉っぱの生成、移動、およびプレイヤーとの当たり判定を更新する
void LeafCutter::LeafUpdate(BlockMap& block, Player* player)
{
	if (Master::gCurrentStageIndex != 0)
	{
		return;
	}

	LeafDebug();

	// スポーン停止フラグが立っていない場合、新しい葉っぱを生成する
	if (!mbStopSpawning && GetRand(100) < LeafSpawn)
	{
		LeafParticle p;

		// 画面のスクロール位置を基準にランダムな水平位置でスポーンさせる
		p.x = static_cast<float>(block.GetScrollX() +
			GetRand(ScreenSize::ScrrenWidth + kSpawnWidthMargin) - kSpawnOffsetLeft);
		p.y = block.GetScrollY() - kSpawnTopOffset;

		// 落下速度や揺れ具合などの物理パラメータをランダムに決定する
		p.fallSpeed = kFallSpeedBase + (GetRand(static_cast<int>(kFallSpeedRandom)) / kPhaseScale);
		p.swaySpeed = kSwaySpeedBase + (GetRand(static_cast<int>(kSwaySpeedRandom)) / 1000.0f);
		p.swayAmplitude = kSwayAmpBase + (GetRand(static_cast<int>(kSwayAmpRandom)) / kPhaseScale);
		p.phase = GetRand(628) / kPhaseScale;
		p.rotation = GetRand(628) / kPhaseScale;
		p.rotationSpeed = (GetRand(static_cast<int>(kRotationSpeedBase * 2.0f)) - kRotationSpeedBase) / kRotationSpeedScale;
		p.scale = kScaleBase + (GetRand(static_cast<int>(kScaleRandom)) / kPhaseScale);
		p.isShadow = (GetRand(100) < kShadowProbability);

		// 特定の位置を超えた場合は生成を行わない
		if (!(p.x > kMaxSpawnLimitX && !mbStopSpawning))
		{
			mLeaves.push_back(p);
		}
	}

	// 既存の葉っぱの位置や回転を更新し、画面外に出たものを削除する
	for (auto it = mLeaves.begin(); it != mLeaves.end(); )
	{
		it->phase += it->swaySpeed;

		// スケールに応じた風の影響度を計算する
		float currentWind = (1.0f - it->scale);
		float curSway = it->swayAmplitude + abs(currentWind) * 0.1f;

		// サイン波を利用して左右に揺れながら落下させる
		it->x += static_cast<float>(sin(it->phase) * curSway + currentWind);

		float swoopY = static_cast<float>(sin(it->phase * kSwoopPhaseFactor) * abs(currentWind) * kSwoopAmpFactor);
		it->y += it->fallSpeed + abs(currentWind) * 0.05f + swoopY;

		// 回転量を更新する
		it->rotation += it->rotationSpeed + (currentWind * kWindScaleFactor);

		// 画面外（上下左右の範囲外）に外れた葉っぱはリストから削除する
		if (it->y > block.GetScrollY() + ScreenSize::ScrrenHeight + kDeleteMarginBottom ||
			it->x < block.GetScrollX() - kDeleteMarginSide ||
			it->y < block.GetScrollY() - kDeleteMarginTop)
		{
			it = mLeaves.erase(it);
		}
		else
		{
			++it;
		}
	}
	LeafCollision(player);
}

void LeafCutter::LeafCollision(Player* player)
{
	if (Master::gCurrentStageIndex != 0 && player->GetPosition().y <= 8000)
	{
		return;
	}

	// プレイヤーが生存している場合、葉っぱとの当たり判定をチェックする
	if (player && !player->GetIsDead())
	{
		VECTOR ppos = player->GetPosition();
		float pLeft = ppos.x - kPlayerHalfWidth;
		float pRight = ppos.x + kPlayerHalfWidth;
		float pTop = ppos.y - kPlayerHalfHeight;
		float pBot = ppos.y + kPlayerHalfHeight;

		for (const auto& leaf : mLeaves)
		{
			// 影以外の実体を持つ葉っぱのみ当たり判定を行う
			if (!leaf.isShadow)
			{
				float leafLeft = leaf.x - kLeafHalfSize;
				float leafRight = leaf.x + kLeafHalfSize;
				float leafTop = leaf.y - kLeafHalfSize;
				float leafBot = leaf.y + kLeafHalfSize;

				// プレイヤーと葉っぱの矩形が交差しているか判定する
				if (leafRight > pLeft && leafLeft < pRight && leafBot > pTop && leafTop < pBot)
				{
					// 葉っぱによるダメージを適用して死亡原因を設定する
					player->SetDeathReason(DeathReason::DeathType::Leaf);
					player->TakeDamage(player->GetHp());
					break;
				}
			}
		}
	}
}