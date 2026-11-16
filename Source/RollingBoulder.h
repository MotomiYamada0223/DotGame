#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;
class Camera;

// 洞窟ステージの巨大転がり岩
// 1. 岩は落下中（天井の穴から床に落ちるまで）のみ当たり判定があり、直撃すると即死する。
// 2. 岩が床を着地して転がっている間は当たり判定がない（プレイヤーを騙すブラフ）。
class RollingBoulder
{
public:
	RollingBoulder();
	~RollingBoulder();

	// 初期化と画像読み込み
	void Initialize();
	// ステージリスタート時の位置・状態リセット
	void Reset();
	// 毎フレームの更新（岩の物理、当たり判定）
	void Update(BlockMap& blockMap, Player* player);
	// 描画（回転岩、土煙パーティクル）
	void Draw(int scrollX, int scrollY);

	bool IsActive() const { return mbActive; }

	// くぼみ手前（X >= 4190.0f）に岩が到達したかを判定（BoulderNeedleTrap用）
	bool HasPassedRecess() const { return (mState == State::Rolling && mX >= 4190.0f); }

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
	int mRollTimer = 0;       // 転がり経過フレーム（SEタイミング制御用）
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

	// 土煙パーティクル生成関数
	void SpawnDust(float x, float y, int count, float speedMul = 1.0f);
};
