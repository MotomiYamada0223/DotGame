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
	static const int PlayerCollisionHeight = 100; // プレイヤーの高さ

	static const int PlayerAttackWidth = 60; // プレイヤーの攻撃範囲の幅
	static const int PlayerAttackHeight = 40; // プレイヤーの攻撃範囲の高さ

	static const int MaxHp = 3; // プレイヤーの最大HP

	// 落下でプレイヤーが死亡する位置
	static const int PlayerDeathHeight = ScreenSize::ScrrenHeight + 100;
}


// キャラクター画像関係
namespace CharacterGraphPath
{
	static const std::string PlayerAnimation = "Resource/Image/player_move.png"; // プレイヤー画像
	static const std::string LivesIcon = "Resource/Image/playerIcon"; // 残機で表示する画像

	// 敵画像
	static const std::string Skeleton = "Resource/Image/SampleSkeleton.png"; // スケルトン画像
	static const std::string Dragon = "Resource/Image/enemy_dragon_move.png"; // dragon画像

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
	static const char* Background = "Resource/Map/backGround.png";
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
namespace BlockCollisionColor
{
	// 赤  Block 当たり判定
	static constexpr int BLOCK_R = 255;
	static constexpr int BLOCK_G = 0;
	static constexpr int BLOCK_B = 0;

	// 青 Death
	static constexpr int DEATH_R = 0;
	static constexpr int DEATH_G = 0;
	static constexpr int DEATH_B = 255;

	// 緑 Goal
	static constexpr int GOAL_R = 0;
	static constexpr int GOAL_G = 255;
	static constexpr int GOAL_B = 0;
}

// プレイヤーがブロックマップとの当たり判定で使用するサイズ
namespace PlayerBlockCollision
{
	// 壁判定時に床や天井のブロックを誤検出しないように上下を縮める
	static constexpr float WallCheckMargin = 20.0f;
	// 壁や床、天井の判定を行う際の厚み
	static constexpr float CollisionThickness = 1.0f;
}

// 表示するテキストの位置
namespace TextPosition
{
	// 落ちた時のGAMEOVER文字
	static const int FallDeathX =  200;
	static const int FallDeathY = ScreenSize::CenterY - 150;

	// 残機表示
	static const int LivesX = ScreenSize::CenterX - 80;
	static const int LivesY = FallDeathY + 200;
}

namespace TextTimer
{
	static constexpr float MaxFallDeathTimer = 180.0f;
}

// マップスクロール関係
namespace MapScrollConstants
{
	// プレイヤーがこの画面X座標より右に行ったらスクロール
	static constexpr int ScrollStartRightX = ScreenSize::CenterX;
	// プレイヤーがこの画面X座標より左に行ったらスクロール
	static constexpr int ScrollStartLeftX = 500;
	// 1フレームあたりのスクロール速度
	static constexpr int ScrollSpeed = 4;
}