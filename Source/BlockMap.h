#pragma once

#include "DxLib.h"
#include <string>
#include <vector>

class BlockMap
{
public:

    // 当たり判定の色によってタイプを分ける
	enum class CollisionType
	{
		None = 0, // 当たり判定なし
		Block,  // 当たり判定あり
		Death, // 死亡判定あり
		Goal,  // ゴール判定あり
	};


    BlockMap();
    ~BlockMap();

    // 背景画像と当たり判定画像を読み込む
    bool Load(
        const std::string& backgroundPath,
        const std::string& collisionPath
    );

    void Move(int playerScreenX, float moveDirection);
    void Draw();

	// 場所の種類を取得する
	CollisionType GetCollisionType(int x, int y) const;
    int ScreenToMapX(int screenX, int scrollX) const { return screenX + scrollX; }

    // 位置のゲッターセッター
    int ScreenToMapX(int screenX) const;
    int ScreenToMapY(int screenY) const;
    int GetScrollX() const { return mScrollX; }

private:
    int mnBackgroundGraph;   // 背景画像
    int mnCollisionSoftImage; // 当たり判定画像 (描画はしない)

    // 当たり判定画像のサイズ
    int mnCollisionWidth;
    int mnCollisionHeight;

    // タイプのデータをのメンバ変数
    std::vector<CollisionType> mCollisionData;

    // 読み込み済みかのフラグ
    bool mbIsLoaded;

    // 配列のインデックスを取得
    // 2次元座標を1次元配列の番号に変換する。
    int GetCollisionIndex(int x, int y) const { return y * mnCollisionWidth + x; }


private: // スクロール関係
    // 画面の左端がマップ画像の何px目なのか
    int mScrollX;
    int mBackgroundWidth;
    int mBackgroundHeight;
};