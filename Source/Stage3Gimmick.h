#pragma once
#include "DxLib.h"

class BlockMap;
class Player;

// =========================================================================
// Stage 3（ステージ3）専用ギミッククラス
// 1. ボス部屋に入った時に天井から降りてきて後ろへ帰れなくする壁ギミック
// 2. ボス部屋にある4つの浮遊台（上下にふよふよ浮遊し、乗ると震えて1.5秒で落下し、
//    落下中もプレイヤーが足場にしっかり接地したまま一緒に落ち、10秒後に復活する）
// =========================================================================
class Stage3Gimmick
{
public:
	Stage3Gimmick();
	~Stage3Gimmick();

	// ギミック画像の読み込みと状態の初期化
	void Initialize(BlockMap& blockMap);

	// プレイヤー物理移動の前に台のY座標（上下ふよふよ・落下移動）を更新
	void UpdatePlatformsBeforePlayerMove(Player* player);

	// Player::PlayerMove 内で呼ばれる、4つの浮遊台とプレイヤーの着地・追従判定
	void HandlePlayerPlatformCollision(Player& player, VECTOR& position, float& velocityY, bool& isGrounded, bool& mbIsJumping, bool& hasJumped);

	// 毎フレームのギミック更新処理（壁の降下・浮遊台の揺れと落下開始・10秒後復活）
	void Update(BlockMap& blockMap, Player* player);

	// ギミック（降りてくる壁・4つの浮遊台）の描画
	void Draw(const BlockMap& blockMap, Player* player);

	// プレイヤー死亡リトライ時のギミックリセット
	void Reset(BlockMap& blockMap);

	// リソース解放
	void Finalize();

	// ドラゴンの咆哮終了時などにランダムな台を破壊し、10秒後に復活させる
	void DestroyRandomPlatform(BlockMap& blockMap, Player* player);
	void DestroyRandomPlatforms(BlockMap& blockMap, Player* player, int count);
	// 最後の大技などで全ての浮遊台を同時に粉砕する
	void DestroyAllPlatforms(BlockMap& blockMap, Player* player);
	void RestoreAllPlatforms();

	// 吹き飛ばし攻撃を受けた際などに浮遊台への追従状態を強制解除する
	void ForceDetachPlayer(Player* player);

	bool IsGateClosed() const { return mbGateClosed; }
	bool IsGateTriggered() const { return mbGateTriggered; }

private:
	// ---------------------------------------------------------------------
	// ギミック1: 部屋に入ったときに天井から降りてきて退路を塞ぐ壁
	// ---------------------------------------------------------------------
	int   mGateWallGraphHandle;   // 降りてくる壁の画像ハンドル (Stage3_GateWall.png: 36x432)
	bool  mbGateTriggered;        // プレイヤーが通過して壁が降り始めたか
	bool  mbGateClosed;           // 壁が完全に閉まりきったか
	float mGateCurrentBottomY;    // 壁の下端の現在Y座標 (684.0f → 1116.0f)
	float mGateDropSpeed;         // 壁の落下速度
	int   mGateShakeTimer;        // 着地時の画面揺れタイマー

	// 【Stage3.png 5724x1692拡大版 対応】ボス部屋入口(X=3016, 通路Y=940..1262)に合わせてゲート座標を調整
	static constexpr int   GATE_MIN_X     = 2982;
	static constexpr int   GATE_MAX_X     = 3015;
	static constexpr int   GATE_TOP_Y     = 940;
	static constexpr int   GATE_BOTTOM_Y  = 1262;
	static constexpr int   GATE_HEIGHT    = GATE_BOTTOM_Y - GATE_TOP_Y; // 322px
	static constexpr float GATE_TRIGGER_X = 3090.0f; // このX座標を超えたら壁が降りてくる

	// ---------------------------------------------------------------------
	// 【Stage3.png 5724x1692拡大版 対応】ギミック2: 5つの浮遊台
	// ---------------------------------------------------------------------
	static constexpr int PLATFORM_COUNT        = 5;
	static constexpr int PLATFORM_FALL_FRAMES  = 90;  // 1.5秒（60fps × 1.5 = 90フレーム）乗っていると落下
	static constexpr int PLATFORM_RESPAWN_TIME = 600; // 10秒（60fps × 10 = 600フレーム）経過で復活

	enum class PlatformState
	{
		Active,     // 通常状態（上下にふよふよ浮遊・乗るとブルブル震える）
		Falling,    // 1.5秒経過して下へ落下中（乗っているプレイヤーも一緒に接地したまま落ちる）
		Waiting     // 落下後、復活までの10秒カウントダウン中
	};

	struct CrumblingPlatform
	{
		int           minX, minY, maxX, maxY; // マップ上の基準矩形座標
		int           graphHandle;            // 台の画像ハンドル
		PlatformState state;                  // 現在の状態
		int           standTimer;             // プレイヤーが乗っているフレーム数（0 .. 90）
		int           respawnTimer;           // 落下してからの経過フレーム数（0 .. 600）
		float         floatPhase;             // 上下ふよふよ浮遊の位相
		float         floatOffsetY;           // 上下ふよふよ浮遊のYオフセット
		float         fallOffsetY;            // 落下中のYオフセット
		float         fallSpeedY;             // 落下速度
		float         currentTopY;            // 現在フレームの台の上面Y座標
		float         prevTopY;               // 前フレームの台の上面Y座標
		float         deltaY;                 // このフレームでの上下移動量
		float         shakeOffsetX;           // 乗っているときの横揺れオフセット（描画用）
		float         shakeOffsetY;           // 乗っているときの縦揺れオフセット（描画用）
		bool          isPlayerRiding;         // 現在プレイヤーがこの台に乗っているか
		int           fadeAlpha;              // 落下・復活時のアルファ値（0 .. 255）
	};

	CrumblingPlatform mPlatforms[PLATFORM_COUNT];
	int               mRidingPlatformIndex; // プレイヤーが現在乗っている台のインデックス（-1: 乗っていない）

	void SetupPlatformDefinitions();
	void ApplyInitialCollision(BlockMap& blockMap);
};
