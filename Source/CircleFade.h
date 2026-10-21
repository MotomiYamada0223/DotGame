#pragma once
#include <DxLib.h>

// ゲーム開始時のフェードの円の描画
class CircleFade
{
public:
	CircleFade();
	~CircleFade();

	void DrawCircleFade();
	void UpdateCircleFade();

private:
	int mnState;
	int mnTimer;
	bool mbFlag;
};