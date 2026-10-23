#pragma once



class Camera
{
public:
    Camera();
    ~Camera();

    // カメラの位置更新
    void Update(
        int playerScreenX,
        int playerScreenY, 
        float moveDirection,
        float currentSpeed,
        int backgroundWidth, 
        int screenWidth,
        int backgroundHeight,
        int screenHeight);

    // スクロール位置取得
    int GetScrollX() const { return mScrollX; }
    int GetScrollY() const { return mScrollY; }

    // 描画用スクロール座標（画面揺れオフセット適用済み）
    int GetDrawScrollX() const { return mScrollX + mShakeOffsetX; }
    int GetDrawScrollY() const { return mScrollY + mShakeOffsetY; }

    // 画面揺れオフセット取得（背景レイヤー等用）
    int GetShakeOffsetX() const { return mShakeOffsetX; }
    int GetShakeOffsetY() const { return mShakeOffsetY; }

    // 画面揺れ（カメラシェイク）開始
    void StartShake(float power, float ratioX, float ratioY, int durationFrames);

    // 画面揺れ更新処理
    void UpdateShake();

    // スクロール中かどうかのフラグ
    bool GetIsScroll() const { return mIsScrolling; }

    // スクロール位置を直接設定
    void SetScrollX(int scrollX) { mScrollX = scrollX; }
    void SetScrollY(int scrollY) { mScrollY = scrollY; }

    // カメラスクロールの固定（ロック）制御
    void SetLocked(bool locked) { mIsLocked = locked; }
    bool IsLocked() const { return mIsLocked; }

private:
    int mScrollX;
    int mScrollY;
    bool mIsScrolling;
    bool mIsLocked;

    // 画面揺れ関連
    float mShakePower;
    float mShakeRatioX;
    float mShakeRatioY;
    int mShakeDuration;
    int mShakeTimer;
    int mShakeOffsetX;
    int mShakeOffsetY;
};