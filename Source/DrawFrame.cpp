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
	//// 天使のフレームを描画
	//DrawGraph(
	//	UIGraphController::SaintFramePosX,
	//	UIGraphController::FramePosY,
	//	mnSaintFrameHandle,
	//	true
	//);

	int distY = 130;

	// 天使のフレームを描画
	DrawRotaGraph(
		UIGraphController::SaintFramePosX,
		UIGraphController::FramePosY + distY,
		1.2f,
		0.0f,
		mnSaintFrameHandle,
		true
	);

	//// テキストのフレームを描画
	//DrawGraph(
	//	UIGraphController::TextFramePosX,
	//	UIGraphController::FramePosY,
	//	mnTextFrameHandle,
	//	true
	//);

	//  テキストのフレームを描画
	DrawRotaGraph(
		UIGraphController::TextFramePosX,
		UIGraphController::FramePosY + distY,
		1.2f,
		0.0f,
		mnTextFrameHandle,
		true
	);

}