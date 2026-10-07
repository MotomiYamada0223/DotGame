#pragma once

class Camera
{
public:
    Camera();
    ~Camera();

    // カメラの位置を更新するため
    void Update(
        int playerScreenX,
        int playerScreenY, 
        float moveDirection,
        float currentSpeed,
        int backgroundWidth, 
        int screenWidth);

    // スクロール位置を取得するため
    int GetScrollX() const { return mScrollX; }
    int GetScrollY() const { return mScrollY; }

    // スクロール中華を取得するゲッター
    bool GetIsScroll() const { return mIsScrolling; }

    // スクロール位置を直接設定するため
    void SetScrollX(int scrollX) { mScrollX = scrollX; }
    void SetScrollY(int scrollY) { mScrollY = scrollY; }

private:
    int mScrollX;
    int mScrollY;
    bool mIsScrolling;
};