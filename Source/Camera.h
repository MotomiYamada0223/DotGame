#pragma once

class Camera
{
public:
    Camera();
    ~Camera();

    // カメラの位置更新
    void Update(
        int playerScreenX,
        float playerMapY,
        float moveDirection,
        float currentSpeed,
        int backgroundWidth,
        int screenWidth,
        int backgroundHeight,
        int screenHeight);

    // スクロール位置取得
    int GetScrollX() const { return mScrollX; }
    int GetScrollY() const { return mScrollY; }

    // 描画用スクロール座標（画面揺れ適用済み・画像の範囲内に収める）
    int GetDrawScrollX() const { return LimitValue(mScrollX + mShakeOffsetX, 0, mMaxScrollX); }
    int GetDrawScrollY() const { return LimitValue(mScrollY + mShakeOffsetY, 0, mMaxScrollY); }

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
    void SetScrollX(int scrollX)
    {
        mScrollX = scrollX;
        mScrollXf = static_cast<float>(scrollX);
    }
    void SetScrollY(int scrollY)
    {
        mScrollY = scrollY;
        mScrollYf = static_cast<float>(scrollY);
    }

    // カメラスクロールの固定（ロック）制御
    void SetLocked(bool locked) { mIsLocked = locked; }
    bool IsLocked() const { return mIsLocked; }
    void SetVerticalLocked(bool locked) { mIsVerticalLocked = locked; }
    bool IsVerticalLocked() const { return mIsVerticalLocked; }

    // 初回フレームでカメラ高さを即座に合わせる
    void ResetScrollInit() { mNeedScrollInit = true; }

    // 右端側への追加スクロール余白（洞窟ボスクリア後の外光落下用）
    void SetRightScrollExtra(int extra) { mRightScrollExtra = (extra > 0) ? extra : 0; }
    int GetRightScrollExtra() const { return mRightScrollExtra; }
    void SetMaxScroll(int maxX, int maxY)
    {
        mMaxScrollX = (maxX > 0) ? maxX : 0;
        mMaxScrollY = (maxY > 0) ? maxY : 0;
    }

private:
    // clamp の代わりに使う範囲制限関数
    template <typename T>
    static T LimitValue(T value, T minValue, T maxValue)
    {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }

private:
    float mScrollXf = 0.0f;
    float mScrollYf = 0.0f;

    int mScrollX;
    int mScrollY;
    bool mIsScrolling;
    bool mIsLocked;
    bool mIsVerticalLocked = false;

    // 最大スクロール量（Update で毎回更新）
    int mMaxScrollX = 0;
    int mMaxScrollY = 0;
    int mRightScrollExtra = 0;

    // 画面揺れ関連
    float mShakePower;
    float mShakeRatioX;
    float mShakeRatioY;
    int mShakeDuration;
    int mShakeTimer;
    int mShakeOffsetX;
    int mShakeOffsetY;

    bool mNeedScrollInit = true;
};