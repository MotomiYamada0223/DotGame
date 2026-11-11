#pragma once
#include "DxLib.h"
#include "GameConstants.h"
#include <vector>

class BlockMap;
class Player;

// 洞窟ステージ（Stage 1-2）の温泉ギミック
// トロッコエリアを越えた先のくぼみ全体（X: 6185～6508, Y: 1905～2006）に配置
// 1. 波打つ水面（リアルタイムSine波シミュレーション）
// 2. 湯気パーティクル・気泡
// 3. プレイヤーが入ると1.5秒（90フレーム）ごとに半ハート回復
// 4. ぬめりが強すぎて1回のジャンプでは上がれず、下に引きずり込まれる。連打で脱出可能！
// 5. 4秒（240フレーム）以上浸かると熱中症で即死！
class HotSpring
{
public:
	HotSpring();
	~HotSpring();

	void Initialize();
	void Reset();
	void Update(BlockMap& blockMap, Player* player);
	void Draw(int scrollX, int scrollY);

private:
	// 温泉の配置範囲（地形のくぼみ・床面に隙間なく合致）
	const float mLeftX = HotSpringConstants::LeftX;
	const float mRightX = HotSpringConstants::RightX;
	const float mBaseWaterY = HotSpringConstants::BaseWaterY; // 水面高さ
	const float mFloorY = HotSpringConstants::FloorY;         // 地面底面（床上面 Y=2006）

	int mWaveTimer = 0;
	int mHeatTimer = 0;
	int mHealPulseTimer = 0;
	bool mbPlayerInside = false;
	float mPlayerX = 0.0f;
	float mPlayerY = 0.0f;

	// パーティクル（湯気・気泡・回復・沸騰煙・汗・水しぶき）
	struct Particle {
		float x, y;
		float vx, vy;
		float size;
		int life;
		int maxLife;
		unsigned int color;
		int type; // 0: 湯気, 1: 気泡, 2: 回復クロス(+), 3: 回復ハート, 4: キラキラ, 5: 沸騰蒸気/火, 6: 水しぶき
	};
	std::vector<Particle> mParticles;

	// 浮遊テキスト（+0.5 HP, あつい！, 熱中症！）
	struct FloatingText {
		float x, y;
		float vy;
		int life;
		int maxLife;
		char text[32];
		unsigned int color;
		unsigned int outlineColor;
	};
	std::vector<FloatingText> mFloatingTexts;

	void SpawnSteam(float x, float y, bool boiling = false);
	void SpawnBubble(float x, float y);
	void SpawnSparkle(float x, float y, unsigned int color);
	void SpawnSplash(float x, float y);
	void SpawnText(float x, float y, const char* str, unsigned int color, unsigned int outlineColor);
	void TriggerHealBurst(float px, float py);
	void TriggerBoilDeath(BlockMap& blockMap, float px, float py);
};
