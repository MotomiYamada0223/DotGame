#include "DrawFrame.h"
#include "GameConstants.h"

DrawFrame::DrawFrame()
{
	// 天使のフレーム画像
	mnSaintFrameHandle = LoadGraph(UIGraphPath::SaintFrame.c_str());
	if (mnSaintFrameHandle == -1) printfDx("天使の背景フレームがない");

	// テキストのフレーム画像
	mnTextFrameHandle = LoadGraph(UIGraphPath::TextFrame.c_str());
	if (mnTextFrameHandle == -1) printfDx("天使の背景フレームがない");
}

DrawFrame::~DrawFrame()
{
	DeleteGraph(mnSaintFrameHandle);
	DeleteGraph(mnTextFrameHandle);
}

void DrawFrame::Draw()
{
	// 天使のフレームを描画
	DrawGraph(
		UIGraphPosition::SaintFramePosX,
		UIGraphPosition::FramePosY,
		mnSaintFrameHandle,
		true
	);
	// テキストのフレームを描画
	DrawGraph(
		UIGraphPosition::TextFramePosX,
		UIGraphPosition::FramePosY,
		mnTextFrameHandle,
		true
	);
}