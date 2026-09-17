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

	static const int MaxHp = 3; // プレイヤーの最大HP
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

	// ハートの画像
	static const std::string HeartFull = "Resource/Image/heart_full.png";
	static const std::string HeartHalf = "Resource/Image/heart_half.png";
	static const std::string HeartEmpty = "Resource/Image/heart_empty.png";
}

// 背景画像の読み込み
namespace BackGroundGraphPath
{
	static const std::string SelectStageMap = "Resource/Image/SampleStageMap.png";
}


// ブロックマップ関係
namespace BlockMapGraphPath
{
	//static const char* Background = "Resource/Map/background.png";
	static const char* Background = "Resource/Map/collision.png";
	static const char* Collision = "Resource/Map/collision.png";
}

// 読み込むCSVパス
namespace CsvPath
{
	static const std::string TutorialText = "Resource/TutorialText/tutorialTextData.csv"; // チュートリアルテキストのCSV
}

// よく使う色
namespace ColorOption
{
	static const int White = GetColor(240, 240, 240);
	static const int Black = GetColor(0, 0, 0);
}

// ダメージの定義
namespace SetDamage
{
	static const int SpikeBlock = 3; // 針と当たったら3ダメージ
	static const int EnemyAttack = 1; // 敵と当たったら1ダメージ
}

// ブロックマップの当たり判定の色の指定
namespace
{
	// 赤 Block
	constexpr int BLOCK_R = 255;
	constexpr int BLOCK_G = 0;
	constexpr int BLOCK_B = 0;


	// 青 Death
	constexpr int DEATH_R = 0;
	constexpr int DEATH_G = 0;
	constexpr int DEATH_B = 255;


	// 緑 Goal
	constexpr int GOAL_R = 0;
	constexpr int GOAL_G = 255;
	constexpr int GOAL_B = 0;
}
