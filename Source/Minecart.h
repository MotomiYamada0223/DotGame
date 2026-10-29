#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;
class Player;

// 洞窟ステージ（Stage 1-2）のトロッコギミック
// 1. プレイヤー初期位置付近（X=6050）で待機
// 2. プレイヤー接近（X >= 5380）で線路（左）へゆっくり走行
// 3. 赤色レール（X=5746）を踏むと超加速！三角台を突き破りながら直進突進
// 4. 三角台は物理破壊（当たり判定消滅・背景マスク・瓦礫飛散）
// 5. プレイヤーが線路を進んでトロッコ初期位置付近（X >= 6000）に来ると、
//    画面左後方から超スピードで反転して戻ってくる！（背後挟み撃ちトラップ）
// 6. プレイヤーの近く（背後約230px）に来るとトロッコは大ジャンプ！
//    そのまま歩いていれば頭上を飛び越えて勝手に避けられるが、
//    ビビってジャンプすると空中のトロッコに直撃して死ぬ！
class Minecart
{
public:
	Minecart();
	~Minecart();

	void Initialize();
	void Reset();
	void Update(BlockMap& blockMap, Player* player);
	void Draw(int scrollX, int scrollY);

	bool IsActive() const { return mbActive; }
	float GetX() const { return mX; }
	float GetY() const { return mY; }

private:
	enum class State {
		Waiting,         // 待機中（X=6050）
		RollingLeft,     // 線路へ向かってゆっくり走行（左向き）
		SuperBoostLeft,  // 赤レールを踏んで超加速！三角台破壊＆左へ突進
		LurkingLeft,     // 画面左外で待機。プレイヤーの進行を待つ
		ReturningRight,  // プレイヤーがX=6000到達で反転猛スピード突進！（背後襲撃＆ジャンプ）
		Finished         // 画面右外へ走り去り終了
	};

	State mState = State::Waiting;
	float mX = 5840.0f;             // 基準X座標（プレイヤー初期位置付近）
	float mY = 1871.0f;             // レール上の基準Y座標
	float mVx = 0.0f;               // 水平走行速度
	float mVy = 0.0f;               // 垂直速度（ジャンプ用）
	float mAngle = 0.0f;            // 描画角度
	bool mbActive = true;           // 有効フラグ
	bool mbSmashedTriangle = false; // 三角台破壊フラグ
	bool mbHasJumped = false;       // 戻りジャンプ実行済みフラグ
	int mGraphHandle = -1;          // トロッコ画像
	int mBobTimer = 0;              // ガタゴト振動タイマー

	// パーティクル（火花・ブースト炎・瓦礫破片）
	struct CartParticle {
		float x, y;
		float vx, vy;
		int size;
		int life;
		int maxLife;
		unsigned int color;
	};
	std::vector<CartParticle> mParticles;

	void SpawnWheelSparks(float x, float y, bool facingLeft);
	void SpawnBoostFlames(float x, float y, bool facingLeft);
	void SpawnRockDebris(float x, float y, int count);
};
