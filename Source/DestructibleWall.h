#pragma once

#include "DxLib.h"
#include <functional>
#include <string>
#include <vector>
#include "BlockEffect.h"

// 攻撃で削れる壁（洞窟ステージの破壊可能壁）を、1枚の矩形単位で管理するクラス
class DestructibleWall
{
public:
    // 削られたマップ座標
    struct Point
    {
        int x, y;
    };

    DestructibleWall() = default;
    ~DestructibleWall();

    // コピーは禁止
    DestructibleWall(const DestructibleWall&) = delete;
    DestructibleWall& operator=(const DestructibleWall&) = delete;


    // マップ上の矩形と画像パスを指定して読み込む
    bool Load(int minX, int minY, int maxX, int maxY, const std::string& imagePath);

    // 攻撃矩形［left, top, right, bottom］と重なる壁ピクセルを削る
    // isBreakable(x, y) が true の座標だけを削り、削った座標の一覧を返す
    std::vector<Point> BreakArea(float left, float top, float right, float bottom,
        const std::function<bool(int, int)>& isBreakable);

    // 毎フレーム呼ぶ：削れた内容をテクスチャへ反映し、破片を更新する
    void Update();

    // スクロール量を引いた画面座標で描画する
    void Draw(int scrollX, int scrollY) const;

private:

    void Release();

    int mMinX = 0, mMinY = 0, mMaxX = 0, mMaxY = 0;
    int mImageWidth = 0, mImageHeight = 0;
    int mSoftImage = -1;     // CPU側のピクセル操作用
    int mGraphHandle = -1;   // 描画用テクスチャ
    bool mIsDirty = false;   // ソフトイメージが変更され、テクスチャ再転送が必要か

    BlockEffect mBlockEffect;
};