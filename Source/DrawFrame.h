#pragma once
#include <DxLib.h>

class DrawFrame
{
public:
	DrawFrame();
	~DrawFrame();

	void Draw();

private:
	int mnSaintFrameHandle; // 天使のフレームハンドル
	int mnTextFrameHandle; // テキストのフレームハンドル
};
