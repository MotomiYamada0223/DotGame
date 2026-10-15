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

	static const int MaxLive = 3; // プレイヤーの最大残機

	// 落下でプレイヤーが死亡する位置
	static const int PlayerDeathHeight = ScreenSize::ScrrenHeight + 100;
}

// UI画像
namespace UIGraphPath
{
	static const std::string SaintFrame = "Resource/Frames/saint_frame.png"; // 天使のフレーム画像
	static const std::string TextFrame = "Resource/Frames/text_frame.png"; // テキストのフレーム画像
}

// UI画像関係
namespace UIGraphController
{
	static const int SaintFramePosX = 500; // 天使のフレーム位置
	static const int TextFramePosX = 695; // テキストのフレーム位置
	static const int FramePosY = 35; // フレーム位置Y

	static const int LifePosX = 120; // 体力はーとの位置X
	static const int LifePosY = 120; // 体力はーとの位置Y
	static const int LifeDistance = 130; // ハートの間隔
	static constexpr float LifeScale = 4.5f; // ハートの拡大率
}


// 天使の位置
namespace SainController
{
	static const int PosX = UIGraphController::SaintFramePosX + 95; // 天使の位置X
	static const int PosY = UIGraphController::FramePosY + 90; // 天使の位置Y
	static constexpr float Scale = 5.5f; // 画像の拡大率
}

// キャラクター画像関係
namespace CharacterGraphPath
{
	static const std::string PlayerAnimation = "Resource/Image/player_move.png"; // プレイヤー画像
	static const std::string LivesIcon = "Resource/Image/playerIcon"; // 残機で表示する画像

	// 敵画像
	static const std::string Skeleton = "Resource/Image/SampleSkeleton.png"; // スケルトン画像
	static const std::string Dragon = "Resource/Enemy/Dragon/enemy_dragon_move.png"; // dragon画像
	static const std::string Needle = "Resource/Enemy/Trap/Needle.png"; // 針の罠画像

	// スライム画像
	static const std::string Slime = "Resource/Enemy/Slime/slime_idle.png"; // スライム画像
	static const std::string SlimeIdle = "Resource/Enemy/Slime/slime_idle.png";
	static const std::string SlimeMove = "Resource/Enemy/Slime/slime_move.png";
	static const std::string SlimeAttack = "Resource/Enemy/Slime/slime_attack.png";

	// キングスライム画像
	static const std::string KingSlimeIdle = "Resource/Enemy/KingSlime/kingSlime_idle.png";
	static const std::string KingSlimeMove = "Resource/Enemy/KingSlime/kingSlime_move.png";
	static const std::string KingSlimeJump = "Resource/Enemy/KingSlime/kingSlime_jump.png";
	static const std::string KingSlimeAttack = "Resource/Enemy/KingSlime/kingSlime_attack.png";


	// 天使の画像
	static const std::string SaintClosed = "Resource/Image/saint_close1.png";
	static const std::string SaintClosedAndOpen = "Resource/Image/saint_close2.png";
	static const std::string SaintOpen = "Resource/Image/saint_open2.png";

	// ギミックの画像
	static const std::string HiddenBlock = "Resource/Image/Block.png";
	static const std::string MoveFloor = "Resource/Image/MoveFloor.png";
	static const std::string MoveFloorBig = "Resource/Image/MoveFloorBig.png";
	static const std::string FeintFloor = "Resource/Image/FeintFloor.png";

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
	static const char* TestBackground = "Resource/Map/TestMap.png";
	static const char* TestCollision = "Resource/Map/TestMapCollision.png";
}

// スクロール背景のパス
namespace ScrollGraphPath
{
	static const char* Stage1 = "Resource/Backgrounds/stage1-1_background.png";
}

// 読み込むCSVパス
namespace CsvPath
{
	static const std::string TutorialText = "Resource/TutorialTexts/tutorialTextData.csv"; // チュートリアルテキストのCSV
	static const std::string TutorialDeathText = "Resource/TutorialTexts/tutorialDeathTextData.csv"; // 死亡テキストのCSV
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

	// 黄 BossArea
	static constexpr int BOSSAREA_R = 255;
	static constexpr int BOSSAREA_G = 255;
	static constexpr int BOSSAREA_B = 0;
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
	static const int FallDeathX = 200;
	static const int FallDeathY = ScreenSize::CenterY - 150;

	// 残機表示
	static const int LivesX = ScreenSize::CenterX - 80;
	static const int LivesY = FallDeathY + 200;
}

// チュートリアルテキストの表示関係
namespace TutorialTextControll
{
	static const int TextX = UIGraphController::SaintFramePosX + 300;
	static const int TextY = UIGraphController::FramePosY + 30;
	static const int TextSize = 45;

	static constexpr float MaxFallDeathTimer = 180.0f;
}

// マップスクロール関係
namespace MapScrollConstants
{
	// プレイヤーがこの画面X座標より右に行ったらスクロール
	static constexpr int ScrollStartRightX = 1000;
	// プレイヤーがこの画面X座標より左に行ったらスクロール
	static constexpr int ScrollStartLeftX = 600;
	static constexpr int ScrollStartUpY = 500;

	// 背景を少し遅くさせるための倍率
	static constexpr float BackgroundScrollSpeedScale = 0.3f;
	// ブロックマップの縦移動を遅くする
	static constexpr float BlockMapSpeedScale = 0.9f;
}

namespace TextTimer
{
	static constexpr float MaxFallDeathTimer = 180.0f;
}

// 動く床関係
namespace MovingFloorConstants
{
	// プレイヤーが床に乗る判定の許容判定距離
	static constexpr float RideMarginTop = 10.0f;    // 上側の許容範囲（めり込み許容）
	static constexpr float RideMarginBottom = 30.0f; // 下側の許容範囲（高速落下時のすり抜け防止）

	// 乗ったら速度が上がる罠で上昇する速度の倍率
	static constexpr float SpeedUpMultiplier = 3.0f;

	// 一定方向に移動し続けるタイプで画面外と判定して破棄するまでの距離（高さのみ）
	static constexpr float DestroyMarginY = 200.0f;

	// 追加: ReverseAndDash用の定数
	static constexpr float ReverseDashMultiplier = 10.0f;
	static constexpr float SpawnYTop = -100.0f;
	static constexpr float SpawnYBottom = 1180.0f;
}

// フェイント床（避け罠）用の定数
namespace FeintFloorConstants
{
	static constexpr float TriggerDistanceX = 80.0f;  // 反応するX軸の距離
	static constexpr float TriggerDistanceY = 155.0f; // 反応するY軸の高さ
	static constexpr float EvadeSpeed = 20.0f;        // 避ける時のスライド速度
	static constexpr float MaxEvadeDistance = 160.0f; // 避ける総距離
	static constexpr float CollisionTopOffset = 30.0f; // 当たり判定の上面を削る量
}

// スライム用の定数
namespace SlimeConstants
{
	static constexpr float CollisionWidth = 70.0f;  // 当たり判定の横幅（見た目110pxに対して適正化）
	static constexpr float CollisionHeight = 65.0f; // 当たり判定の縦幅（見た目105pxに対して適正化）
}

// ボス戦開始演出用の定数
namespace BossIntroConstants
{
	static constexpr int LetterboxHeight = 50;   // 黒帯の最大高さ(px)
	static constexpr int LetterboxAlpha  = 255;  // 黒帯の最大不透明度
	static constexpr int FadeFrames      = 60;   // 黒帯フェードインのフレーム数（約1秒）
	static constexpr float AutoWalkSpeed = 3.0f; // 演出中の自動歩行速度
}

// キングスライム出現演出・警告用定数
namespace KingSlimeIntroConstants
{
	static constexpr float Scale                 = 4.0f;  // キングスライムの拡大率
	static constexpr float HitBoxScale			 = 3.0f;  // キングスライムの当たり判定の拡大率
	static constexpr int WaitFramesBeforeWarning = 45;    // 到着後の待機フレーム（約0.75秒）
	static constexpr int WarningDurationFrames   = 75;    // 警告表示フレーム（約1.25秒）
	static constexpr int WarningFadeOutFrames    = 15;    // 警告フェードアウトフレーム（約0.25秒）
	static constexpr float WarningWidth          = 80.0f * Scale; // 警告の横幅（キングスライム当たり判定幅準拠: 320.0f）
	static constexpr float WarningHeight         = 450.0f;// 警告の縦幅
	static constexpr float EvasionSpeedX         = 8.0f; // 回避ジャンプの水平移動速度
	static constexpr float EvasionJumpSpeedY     = -10.0f;// 回避ジャンプの初速Y（上方向）
	static constexpr int EvasionDurationFrames   = 22;    // 回避ジャンプの持続フレーム数
}

// キングスライム着地時の画面揺れ用定数
namespace KingSlimeShakeConstants
{
	static constexpr float ShakePower     = 30.0f; // 揺れの最大強度（ピクセル）
	static constexpr float RatioY         = 0.8f;  // 縦方向の揺れ割合（8割）
	static constexpr float RatioX         = 0.2f;  // 横方向の揺れ割合（2割）
	static constexpr int   DurationFrames = 30;    // 揺らすフレーム数
}

