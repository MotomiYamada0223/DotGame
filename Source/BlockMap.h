#pragma once

#include "DxLib.h"
#include <string>
#include <vector>
#include "Camera.h"
#include "DestructibleWall.h"
#include <memory>

class BlockMap
{
public:

    // 当たり判定の色によってタイプを分ける
	enum class CollisionType : unsigned char
	{
		None = 0, // 当たり判定なし
		Block,  // 当たり判定あり
		Death, // 死亡判定あり
		Goal,  // ゴール判定あり
		BossArea, // ボスエリア判定あり
        BreakableBlock, // 破壊可能な壁
	};

    // 攻撃矩形と重なる破壊可能壁を削る。1ピクセルでも削れたら true
    bool BreakWallAtArea(float left, float top, float right, float bottom);

    BlockMap();
    ~BlockMap();


    // 背景画像と当たり判定画像を読み込む
    bool Load(
        const std::string& backgroundPath,
        const std::string& collisionPath
    );
    void Unload();

    void Move(int playerScreenX, float playerMapY, float moveDirection, float currentSpeed);
    void Draw();
    void DebugDraw();

	// 場所の種類を取得する
	CollisionType GetCollisionType(int x, int y) const;
	// 指定した矩形範囲の当たり判定を一括で上書きする（隠しブロック等用）
	void SetCollisionBlock(int startX, int startY, int width, int height, CollisionType type);
    void ClearCollisionArea(int minX, int minY, int maxX, int maxY);
    void FillCollisionArea(int minX, int minY, int maxX, int maxY, CollisionType type);
    int ScreenToMapX(int screenX, int scrollX) const { return screenX + scrollX; }

    // 位置のゲッターセッター
    int ScreenToMapX(int screenX) const;
    int ScreenToMapY(int screenY) const;
    int GetScrollX() const { return mCamera.GetScrollX(); }
    int GetScrollY() const { return mCamera.GetScrollY(); }
    void SetScrollX(int scrollX) { mCamera.SetScrollX(scrollX); }
    void SetScrollY(int scrollY) { mCamera.SetScrollY(scrollY); }
    int GetDrawScrollX() const { return mCamera.GetDrawScrollX() + mShakeX; }
    int GetDrawScrollY() const { return mCamera.GetDrawScrollY() + mShakeY; }
    int GetShakeOffsetX() const { return mCamera.GetShakeOffsetX() + mShakeX; }
    int GetShakeOffsetY() const { return mCamera.GetShakeOffsetY() + mShakeY; }

    void StartCameraShake(float power, float ratioX, float ratioY, int durationFrames)
    {
        
        mCamera.StartShake(power, ratioX, ratioY, durationFrames);
    }

    void SetCameraLocked(bool locked) { mCamera.SetLocked(locked); }
    void LockScroll(bool locked) { mCamera.SetLocked(locked); }
    bool IsCameraLocked() const { return mCamera.IsLocked(); }
    void SetVerticalScrollLocked(bool locked) { mCamera.SetVerticalLocked(locked); }
    void SetAllowScrollPastRight(bool allow);
    void SetAllowRightEdgeFall(bool allow) { SetAllowScrollPastRight(allow); }

    // 背景画像の元の長さを取得する プレイヤーの移動領域のため
    int GetCurrentWidth() const { return mnBackgroundWidth; }
    int GetCurrentHeight() const { return mnBackgroundHeight; }

    // ステージ3ギミック等の画面揺れ＆破片パーティクル用
    struct DebrisParticle
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        int size = 4;
        int life = 20;
        int maxLife = 20;
        unsigned int color = 0;
    };
    std::vector<DebrisParticle> mDebris;
    int mShakeX = 0;
    int mShakeY = 0;

    // スクロール中かの判断
    bool GetIsScrolling() const { return mCamera.GetIsScroll(); }
    float GetScrollDirection() const { return mfMoveDirection; }

    // スクロールリセット
    void ResetScroll();

    // カメラを指定ワールド座標に瞬時にスナップ（カメラスクロール領域内にプレイヤーが収まるように移動）
    void SnapCameraToPosition(float targetWorldX, float targetWorldY);

    // 洞窟ボス登場時の天井破砕演出
    void InitCeilingBreak(float centerWorldX);
    void InitBrokenCeilingHoleImmediate(float centerWorldX);
    void ProgressCeilingBreak(float dragonBottomWorldY, float dragonCenterWorldX);
    void UpdateCeilingBreak();
    void DrawCeilingForegroundDebris() const;
    void ResetCeilingBreak();
    bool IsCeilingBreakInitialized() const { return mbCeilingBreakActive; }
    float GetCeilingBreakCenterX() const { return mCeilingBreakCenterX; }

    // 洞窟オープニング（Cave_Opening）の崩落穴・出口光演出
    void InitOpeningPitBreak();
    void SetOpeningPitAlreadyBroken();
    void ResetOpeningPitBreak();
    bool CheckAndTriggerOpeningPit(float playerX, float playerY, float playerWidth, float playerHeight, bool isGrounded, float velocityY);
    void TriggerOpeningPitBreak(float playerWorldX);
    void UpdateOpeningPitBreak();
    bool IsOpeningMapMode() const { return mbOpeningMapMode; }
    bool IsOpeningPitBroken() const { return mbOpeningPitBroken; }
    int GetOpeningPitLeft() const { return mOpeningPitLeft; }
    int GetOpeningPitRight() const { return mOpeningPitRight; }
    int GetOpeningPitTop() const { return mOpeningPitTop; }

    // ボスエリア関連
    bool HasBossArea() const { return mbHasBossArea; }
    float GetBossAreaCenterX() const {
        if (!mbHasBossArea) return 0.0f;
        return (static_cast<float>(mnBossAreaLeft) + static_cast<float>(mnBossAreaRight)) * 0.5f;
    }
    int GetBossAreaLeft() const { return mnBossAreaLeft; }
    int GetBossAreaRight() const { return mnBossAreaRight; }


private:
    int mnBackgroundGraph;   // 背景画像
    int mnCollisionSoftImage; // 当たり判定画像 (描画はしない)

    // 当たり判定画像のサイズ
    int mnCollisionWidth;
    int mnCollisionHeight;

    // タイプのデータをのメンバ変数
    std::vector<CollisionType> mCollisionData;
    Camera mCamera; // カメラのインスタンス

    // 読み込み済みかのフラグ
    bool mbIsLoaded;
    bool mbHasBossArea;
    int mnBossAreaLeft;
    int mnBossAreaRight;
    bool mbHasBreakableBounds;
    int mnBreakableMinX;
    int mnBreakableMinY;
    int mnBreakableMaxX;
    int mnBreakableMaxY;

    // 配列のインデックスを取得
    // 2次元座標を1次元配列の番号に変換する。
    int GetCollisionIndex(int x, int y) const { return y * mnCollisionWidth + x; }

    // 破壊可能壁
    std::vector<std::shared_ptr<DestructibleWall>> mWalls;



private: // スクロール関係
    // 画面の左端がマップ画像の何px目なのか
    int mnBackgroundWidth;
    int mnBackgroundHeight;
    bool mbIsScrolling; // スクロール中かを判断
    float mfMoveDirection; // スクロール方向

private: // 洞窟ボス登場時の天井破砕演出用データ
    struct CeilingChunk
    {
        int graphHandle = -1;       // DerivationGraph で切り出した岩塊ハンドル
        float origX = 0.0f;         // 元の左上ワールドX
        float origY = 0.0f;         // 元の左上ワールドY
        int size = 24;              // チャンク1辺のピクセルサイズ
        int width = 24;             // 実際の切り出し横幅
        int height = 24;            // 実際の切り出し縦幅
        bool isTarget = false;      // 破壊対象マスクに含まれるか（ギザギザ境界用）
        bool hadCollision = false;  // 元々Blockコリジョンを持っていたか（リセット復元用）
        bool isBroken = false;      // すでに砕けて落下中か
        bool isFinished = false;    // 地面落下後に消滅完了したか
        bool isForeground = false;  // ドラゴンの手前に描くか、奥に描くか
        float x = 0.0f;             // 現在の中心ワールドX
        float y = 0.0f;             // 現在の中心ワールドY
        float vx = 0.0f;            // X速度
        float vy = 0.0f;            // Y速度
        float angle = 0.0f;         // 回転角(rad)
        float rotSpeed = 0.0f;      // 回転角速度
        float alpha = 255.0f;       // 不透明度
        int bounceCount = 0;        // 床バウンド回数
        float triggerDelayY = 0.0f; // 同一段での砕けタイミングのバラつき
    };

    struct CeilingDust
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float radius = 6.0f;
        float alpha = 200.0f;
        int color = 0;
    };

    struct OpeningLightDust
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float phase = 0.0f;
        int size = 3;
    };

    void DrawBackgroundWithCutout(int sx, int sy, int sw, int sh, int holeLeft, int holeTop, int holeRight, int holeBottom) const;
    void DrawBrokenCeilingChunk(const CeilingChunk& chunk, int sx, int sy) const;
    void DrawOpeningExitLight(int sx, int sy) const;
    static void UpdateDustParticles(std::vector<CeilingDust>& dusts, float gravity, float fadeSpeed);

    bool mbCeilingBreakActive = false;
    int mCeilingRegionGraph = -1;
    int mCeilingHoleLeft = 0;
    int mCeilingHoleTop = 0;
    int mCeilingHoleRight = 0;
    int mCeilingHoleBottom = 0;
    float mCeilingBreakCenterX = 0.0f;
    float mLatestDragonBottomY = -9999.0f;
    std::vector<CeilingChunk> mCeilingChunks;
    std::vector<CeilingDust> mCeilingDusts;

    // 洞窟オープニングの崩落穴・出口光演出用データ
    bool mbOpeningMapMode = false;
    bool mbOpeningPitBroken = false;
    int mOpeningPitRegionGraph = -1;
    int mOpeningPitLeft = 0;
    int mOpeningPitTop = 0;
    int mOpeningPitRight = 0;
    int mOpeningPitBottom = 0;
    int mOpeningLightTimer = 0;
    std::vector<CeilingChunk> mOpeningPitChunks;
    std::vector<CeilingDust> mOpeningPitDusts;
    std::vector<OpeningLightDust> mOpeningLightDusts;
};