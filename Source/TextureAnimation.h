#pragma once
#include"DxLib.h"
#include <string>

class TextureAnimation
{
public:
	TextureAnimation(
		VECTOR position,
		std::string filename,
		int allNum,
		int xNum,
		int yNum,
		int interval
		);

	~TextureAnimation();


	void Update(); // 更新
	void Draw(bool turnFlag = false); // 描画

	// セッター関数 // 
	// ポジション設定
	void SetInterval(int interval) { mnInterval = interval; }
	void SetPosition(VECTOR centerPosition) { mvPosition = centerPosition; }

	// ゲッター関数 //
	// ポジション取得
	VECTOR GetPosition() { return mvPosition; }

	// アニメーション制御
	int GetCurrentFrame() const { return mnCurrentNum; }
	void SetCurrentFrame(int frame) {
		if (frame >= 0 && frame < mnAllNum) {
			mnCurrentNum = frame;
			mnCounter = 0;
			mbFinished = false;
		}
	}
	void ResetAnimation() { mnCurrentNum = 0; mnCounter = 0; mbFinished = false; }
	void SetReverse(bool reverse) { mbReverse = reverse; }
	bool IsReverse() const { return mbReverse; }
	void SetLoop(bool loop) { mbLoop = loop; }
	bool IsFinished() const { return mbFinished; }
	int GetAllNum() const { return mnAllNum; }


	// 拡大率
	void SetScale(float scale) { mfScale = scale; }
	float GetScale() const { return mfScale; }

	// プレイヤーで使うため
	int GetSizeX() { return mnxNum; }  // 幅
	int GetSizeY() { return mnyNum; }  // 高さ

	//　半径取得
	float GetRadius() { return mfRadius; }


private:
	VECTOR mvPosition; // ポジション
	int mnCounter;     // アニメーションカウンタ
	int mnInterval;   // テクスチャ切り替えのフレーム数
	int mnCurrentNum;  // 何番目のテクスチャを表示するか
	int* mnHandleList; // 分割されたテクスチャのハンドルリスト
	int mnAllNum;      // テクスチャ分割数

	float mfRadius;    // 半径
	float mfScale;     // 拡大率

	bool mbReverse;    // 逆再生フラグ
	bool mbLoop;       // ループフラグ
	bool mbFinished;   // 非ループ時の再生完了フラグ

	// メンバ変数の追加
	int mnxNum;
	int mnyNum;

};
