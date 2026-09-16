#pragma once
#include <string>
#include <DxLib.h>


// 画像パスや色などの定義をして他クラスから呼び出す
// PG以外も変更しやすい使用にするため

// スクリーンサイズの定義
namespace ScreenSize
{
	// スクリーンの画面サイズ
	static const int ScrrenWidth = 1920;
	static const int ScrrenHeight = 1080;

	// スクリーンの画面の半分サイズ
	static const int CenterX = ScrrenWidth / 2;
	static const int CenterY = ScrrenHeight / 2;
}

// プレイヤー関係
namespace PlayerConstants
{
	static const int PlayerCollisionWidth = 32; // プレイヤーの幅
	static const int PlayerCollisionHeight = 80; // プレイヤーの高さ

	static const int PlayerAttackWidth = 60; // プレイヤーの攻撃範囲の幅
	static const int PlayerAttackHeight = 40; // プレイヤーの攻撃範囲の高さ
}


// キャラクター画像関係
namespace CharacterGraphPath
{
	static const std::string PlayerAnimation = "Resource/Image/player.png"; // プレイヤー画像

	static const std::string Skeleton = "Resource/Image/SampleSkeleton.png"; // スケルトン画像

	// 天使の画像
	static const std::string SaintClosed = "Resource/Image/saint_close1.png";
	static const std::string SaintClosedAndOpen = "Resource/Image/saint_close2.png";
	static const std::string SaintOpen = "Resource/Image/saint_open2.png";
}

// 背景画像の読み込み
namespace BackGroundGraphPath
{
	static const std::string SelectStageMap = "Resource/Image/SampleStageMap.png";
}


// ブロックマップ関係
namespace BlockMapGraphPath
{
	static const std::string BlockMap = "Resource/Map/image_Map.png"; // ブロックマップの画像
}

// ブロックマップの当たり判定の設定
namespace BlockCollision
{
	static const int MinDamageBlock = 13; // ダメージ判定があるブロックの最小値
	static const int MaxDamageBlock = 15; // ダメージ判定があるブロックの最大値
}

// 読み込むCSVパス
namespace CsvPath
{
	static const std::string BlockMapFile = "Resource/Map/BlockMapFile.csv"; // ブロックマップの配列CSV
	static const std::string TutorialText = "Resource/TutorialText/tutorialTextData.csv"; // チュートリアルテキストのCSV
}

// よく使う色
namespace ColorOption
{
	static const int White = GetColor(240, 240, 240);
	static const int Black = GetColor(0, 0, 0);
}