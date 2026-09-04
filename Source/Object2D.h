#pragma once
#include "DxLib.h"
#include <string>

// クラスの前方宣言
class Texture;
class TextureAnimation;

// 
// 2D オブジェクトの基底クラス
// 2D オブジェクト（プレイヤーや敵など）を作る際は、
// 必ずこれを継承して作成する　
// 
class Object2D
{
public:    // enum, struct, 定数の定義

	// オブジェクトを見分けるためのタグ
	// Object2D だけだと何か分からないあの出見分けるためのタグ
	enum Tag
	{
		// ---★★★---
		// タグの番号被ったらバグるから要注意 
		
		// Titleシーンでつかわれるタグ（1000～）

		// Gameシーンでつかわれるタグ（2000～）
		BattleMap2D  = 2200,    // マップ
		FrontMap  = 2201,    // マップ
		BattlePlayer2D = 2300,  // プレイヤー
		Enemy2D = 2350,         // 敵
		HomingEnemy2D = 2400,   // パックンみたいな敵
		FlyEnemy2D = 2450,      // 飛んでる敵
		Coin2D = 2500,          // コイン
		Item2D = 2550,          // 回復アイテム
		Goal2D = 2600,          // ゴールポイント


		PlayerBullet2D = 3000,      // プレイヤーの弾
		HomingEnemyBullet2D = 3100, // ホーミングしてくる敵の弾
		BulletUP2D = 3200,          // プレイヤーの上向きの弾
		HomingBullet2D = 3300,      // ホーミング弾（使ってない）

	};


public:
	Object2D(std::string filename, VECTOR initPos);

	// コンストラクタ（アニメーション用）
	Object2D(VECTOR initPos, std::string filename, int allNum, int numX, int numY, int interval);
	
	
	// コンストラクタ(Player用)
	Object2D(const VECTOR initPos);




	// ここでマップで定義したゲッターセッターをバーチャルしている（中身は空にしている）
	//virtual void SetDrawSize() { return; }
	//virtual VECTOR GetDrawSize() { return VGet(0.0f, 0.0f, 0.0f); }




	virtual ~Object2D();

	virtual void Update();

	virtual void Draw();

public:   // ゲッター・セッター
	void SetPosition(VECTOR pos) { mvPosition = pos; }  // 座標設定
	VECTOR GetPosition() { return mvPosition; }         // 座標取得

	void SetDeleteFlag(bool flag) { mbDeleteFlag = flag; }  // 削除フラグ設定
	bool IsDeleteFlag() { return mbDeleteFlag; }           // 削除フラグ取得

	void SetTag(Tag tag) { mnTag = tag; }  // タグ設定
	Tag GetTag() { return mnTag; }         // タグ取得


	virtual float GetRadius(); // 半径の取得

	// プレイヤーで使うため
	int GetSizeX();  // 幅
	int GetSizeY();  // 高さ


protected:
	//2Dの要素は何が必要か考えて書く
	Texture* mpTexture;   // 画像
	VECTOR mvPosition;    // 座標
	TextureAnimation* mpTextureAnimation; // アニメーション画像
	
private:
	bool mbDeleteFlag;    // 削除フラグ(これがtrue になっていると自動的に削除される（ように作る）)
	Tag mnTag;            // オブジェクトを見分ける用のタグ

	// プレイヤー用
	VECTOR mvDirection;
};

/*
// 一枚画像用のクラス
class Tecture2D : public Object2D
{

};

// テクスチャアニメーション用のクラス
class TectureAnimation2D : public Object2D
{

};*/