#pragma once
#include <vector>

class Player;
class BlockMap;

class StageClear
{
public:
	enum class State
	{
		Inactive,          
		PreCrackerDelay,   // 液体弾消滅後の待機時間
		CrackerSlideIn,    // クラッカースライドイン
		CrackerAimWait,    // クラッカー停止後の待機
		CrackerBurstWait,  // クラッカー破裂後
		PostBulletDelay,   // 全弾丸消滅後の小休止
		LogoPreFallShake,  // ロゴ落下直前の予兆
		LogoFall,          // タイトルロゴ上空落下
		Blink,             // 「ステージクリア」文字の点滅
		Display,           // 常時表示フェーズ
		ClearCleanUp,      // クラッカー・文字・ロゴの退出・フェードアウト
		DoorAppearWait,    // 扉出現前の静寂（少ししたら）
		DoorAppear,        // 煙演出とともに扉出現（ポップ）
		DoorActive,        // 扉設置完了（プレイヤー接近で開閉）
		Finished           
	};

	struct CrackerData
	{
		float x;
		float y;
		float startX;
		float targetX;
		float targetY;
		float angle;           // 描画回転角
		float shootAngleRad;   // 画面空間における射出方向角
		bool isRightSide;
		float recoilOffset;    // 後退反動オフセット
		float squishScaleX;    // 横方向弾性スケール
		float squishScaleY;    // 縦方向弾性スケール
		float mouthX;          // 発射口X
		float mouthY;          // 発射口Y
	};

	struct ConfettiParticle
	{
		float x;
		float y;
		float vx;
		float vy;
		float angle;           // 進行方向 / 向き角
		float spinAngle;       // 表裏反転回転角
		float spinSpeed;
		float sizeW;
		float sizeH;
		int r, g, b;           // 色成分
		int shapeType;         // 長方形リボン, 星
		int timer;
		int life;
		bool isActive;
	};

	struct SilverBullet
	{
		float x;
		float y;
		float vx;
		float vy;
		float gravity;
		float radius;
		float prevX[3];
		float prevY[3];
		bool isAimed;
		bool isActive;
	};

	struct ImpactDust
	{
		float x;
		float y;
		float vx;
		float vy;
		float size;
		int alpha;
		int life;
		int timer;
		bool isActive;
	};

	struct DoorSmokeParticle
	{
		float x;
		float y;
		float vx;
		float vy;
		float size;
		int alpha;
		int r, g, b;
		int life;
		int timer;
		bool isActive;
	};

	StageClear();
	~StageClear();

	// 演出の開始
	void Start();

	// 毎フレームの更新
	void Update(Player* player = nullptr, BlockMap* pBlockMap = nullptr);

	// 描画
	void Draw() const;

	// ステージ背面描画
	void DrawBehindStage(const BlockMap* pBlockMap = nullptr) const;

	// ステージクリア扉描画（プレイヤー背面・背景前面）
	void DrawDoor(const BlockMap* pBlockMap = nullptr) const;

	// リセット
	void Reset();

	// 状態取得
	State GetState() const { return mState; }
	bool IsActive() const { return mState != State::Inactive && mState != State::Finished; }
	bool IsFinished() const { return mState == State::Finished; }

	// 静的アクセス用
	static StageClear* GetInstance() { return sInstance; }
	static void Trigger() { if (sInstance) sInstance->Start(); }
	static bool IsStageClearActive() { return sInstance != nullptr && sInstance->IsActive(); }

private:
	void InitCrackers();
	void BurstCrackers();
	void TrackPlayerWithCracker(int crackerIdx, Player* player, BlockMap* pBlockMap);
	void UpdateCrackers();
	void UpdateConfetti();
	void UpdateBullets(Player* player, BlockMap* pBlockMap);
	void DrawCrackers() const;
	void DrawConfetti() const;
	void DrawBullets() const;
	void DrawMuzzleFlashes() const;
	void InitLogo();
	void UpdateLogo(Player* player, BlockMap* pBlockMap);
	void DrawLogo(const BlockMap* pBlockMap = nullptr) const;
	void DrawLogoGroundImpact(const BlockMap* pBlockMap = nullptr) const;
	void SpawnImpactDust(float x, float y);
	void UpdateImpactDust();
	void DrawImpactDust() const;
	void SpawnDoorSmoke(float x, float y);
	void UpdateDoorSmoke();
	void DrawDoorSmoke() const;

private:
	State mState;
	int mTimer;            // 現在のフェーズのタイマー
	int mBurstTimer;       // 破裂してからの経過タイマー
	int mCrackerGraph;     // Cracker.png 画像ハンドル
	int mFlashGraphs[5]; // Flash.png 分割画像ハンドル

	int mAimedCrackerIndex;// 狙撃を行うクラッカーのインデックス (0〜5)
	bool mAimLocked;       // 発射0.3秒前の照準ロックフラグ

	std::vector<CrackerData> mCrackers;
	std::vector<ConfettiParticle> mConfetti;
	std::vector<SilverBullet> mBullets;
	std::vector<ImpactDust> mImpactDusts;

	int mnLogoHandle;      // title_logo.png 画像ハンドル
	float mLogoY;          // ロゴ中心Y座標
	float mLogoVy;         // ロゴ落下速度
	float mLogoTargetY;    // 突き刺さり停止目標中心Y座標
	float mLogoScale;      // ステージクリア文字幅に合わせた拡縮率
	bool mbLogoLanded;     // 地面に突き刺さったかのフラグ
	int mLogoImpactTimer;  // 着地後の経過フレーム数
	float mLogoReboundOffset; // 着地時の微小リバウンド振動オフセット

	int mDoorGraphs[4];      // OpenDoor.png 分割画像ハンドル (4コマ)
	float mDoorWorldX;       // 扉ワールドX座標
	float mDoorWorldY;       // 扉接地ワールドY座標
	float mDoorAnimProgress; // 扉開閉アニメーション進行度 (0.0f〜3.0f)
	int mDoorCurrentFrame;   // 現在描画するコマ (0〜3)
	float mDoorPopScale;     // 出現時のポップ弾性拡縮率
	mutable bool mbDoorDrawnThisFrame; // フレーム内での扉描画済みフラグ
	std::vector<DoorSmokeParticle> mDoorSmoke; // 扉出現煙パーティクル

	const BlockMap* mpBlockMap;
	static StageClear* sInstance;
};