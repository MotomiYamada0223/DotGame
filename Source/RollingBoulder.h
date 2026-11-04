#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;
class Camera;

// 洞窟ステージの巨大転がり岩＆単一巨大トゲ発射トラップ
// 1. 岩は落下中（天井の穴から床に落ちるまで）のみ当たり判定があり、直撃すると即死する。
// 2. 岩が床を着地して転がっている間は当たり判定がない（プレイヤーを騙すブラフ）。
// 3. 坂下の2マスのくぼみには横幅2マス分の巨大な1本のトゲが隠されている。
//    - プレイヤーがくぼみに入った時、または岩が転がってくぼみに近づいた時に下から出現する。
//    - プレイヤーがそのくぼみを飛び越えようと空中を通過した瞬間、トゲが上空へミサイルのように発射される！
//    - トゲに当たったプレイヤーは即死する。
class RollingBoulder
{
public:
	RollingBoulder();
	~RollingBoulder();

	// 初期化と画像読み込み
	void Initialize();
	// ステージリスタート時の位置・罠リセット
	void Reset();
	// 毎フレームの更新（岩の物理、トゲの出現・発射、当たり判定）
	void Update(BlockMap& blockMap, Player* player);
	// 描画（回転岩、巨大トゲ、パーティクル）
	void Draw(int scrollX, int scrollY);

	bool IsActive() const { return mbActive; }

private:
	// 岩の状態マシン
	enum class State {
		Waiting,    // プレイヤーが坂道に到達するまで天井上空で待機
		Falling,    // 天井の穴から床へ垂直落下中（★この間のみ即死当たり判定あり）
		Rolling,    // 床および下り坂を回転しながら転走中（★当たり判定なし・ブラフ）
		Pitfall,    // 落とし穴に突入して奈落へ落下中
		Finished    // 落下完了して消滅
	};

	State mState = State::Waiting;
	float mX = 3128.0f;       // 中心X座標（天井穴の中央）
	float mY = -200.0f;       // 中心Y座標（画面外の上空）
	float mVx = 0.0f;         // 水平速度
	float mVy = 0.0f;         // 垂直速度
	float mRadius = 160.0f;   // 岩の半径（直径320px）
	float mAngle = 0.0f;      // 転がり回転角度（ラジアン）
	bool mbTriggered = false; // 起動済みフラグ
	bool mbActive = true;     // アクティブフラグ
	int mGraphHandle = -1;    // 岩の画像ハンドル

	// 岩の土煙・破片パーティクル
	struct BoulderDust {
		float x, y;
		float vx, vy;
		int size;
		int life;
		int maxLife;
		unsigned int color;
	};
	std::vector<BoulderDust> mDustList;

	// くぼみ（X=4440～4515、2マス分）の単一巨大トゲ罠
	enum class SpikeTrapState {
		Hidden,     // くぼみの底に潜伏中
		Emerging,   // 下からせり上がり中（岩の接近、またはプレイヤーの進入で発動）
		Ready,      // くぼみに完全に突き出して待機中（入ると即死）
		Launched,   // 飛び越えようとしたプレイヤーに向けて上空へロケット発射！
		Finished    // 上空へ飛び去り消滅
	};

	SpikeTrapState mSpikeState = SpikeTrapState::Hidden;
	float mSpikeBaseX = 4315.5f;     // トゲの中央X座標（2マスの中央）
	float mSpikeWidth = 72.0f;       // 2マス分の横幅（4440～4515）
	float mSpikeHeight = 0.0f;       // トゲの現在の高さ
	float mSpikeMaxHeight = 115.0f;  // 完全出現時のトゲの高さ
	float mSpikeY = 2006.0f;         // トゲの底面Y座標（発射時は上へ移動）
	float mSpikeVy = 0.0f;           // 上空への発射速度

	// トゲ刺突・発射時のパーティクル
	struct SpikeParticle {
		float x, y;
		float vx, vy;
		int size;
		int life;
		int maxLife;
		unsigned int color;
	};
	std::vector<SpikeParticle> mSpikeParticles;

	// パーティクル生成関数
	void SpawnDust(float x, float y, int count, float speedMul = 1.0f);
	void SpawnSpikeParticles(float x, float y, int count);
};
