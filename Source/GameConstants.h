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

// 音量に関する設定
namespace SoundSetting
{
	static constexpr int VolumeMin = 0;
	static constexpr int VolumeMax = 100;

	// 共通シーンで使用する音量
	static constexpr int DecideVolume = 40; // 決定音
}


// プレイヤー関係
namespace PlayerConstants
{
	static const int PlayerCollisionWidth = 32; // プレイヤーの幅
	static const int PlayerCollisionHeight = 100; // プレイヤーの高さ

	static const int PlayerAttackWidth = 100; // プレイヤーの攻撃範囲の幅
	static const int PlayerAttackHeight = 50; // プレイヤーの攻撃範囲の高さ
	static const int PlayerAttackDist = 10; // プレイヤーからどれだけ離れているか

	static const int MaxLive = 3; // プレイヤーの最大残機

	// 落下でプレイヤーが死亡する位置
	static const int PlayerDeathHeight = 3000;
	// Wキー・Sキー長押しによるステージ裏・表落下判定フレーム数
	static constexpr int FallThroughHoldFrames = 30;

	// 3段階ジャンプ（小・中・大）用定数
	enum class JumpType
	{
		None,
		Small,  // 小ジャンプ
		Mid,    // 中ジャンプ
		Large   // 大ジャンプ
	};

	static constexpr int   SmallJumpMaxHoldFrames = 7;      // 小ジャンプ判定の最大押下フレーム数（〜約0.11秒）
	static constexpr int   MidJumpMaxHoldFrames   = 18;     // 中ジャンプ判定の最大押下フレーム数（〜約0.30秒）
	static constexpr float SmallJumpTargetHeight  = 105.0f; // 小ジャンプの目標頂点高さ（px）
	static constexpr float MidJumpTargetHeight    = 205.0f; // 中ジャンプの目標頂点高さ（px）
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
	static const int FramePosY = 15; // フレーム位置Y

	static const int LifePosX = 120; // 体力はーとの位置X
	static const int LifePosY = 60; // 体力はーとの位置Y
	static const int LifeDistance = 120; // ハートの間隔
	static constexpr float LifeScale = 4.5f; // ハートの拡大率
}

namespace MovingFloorConstants
{
	// 配置位置から上下どちらへこの距離（px）以上離れたら破棄する
	constexpr float DestroyDistance = 1500.0f;
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
	static const std::string BloodNeedle = "Resource/Image/BloodNeedle.png"; // 洞窟ステージの血糊針画像
	static const std::string CaveRamp = "Resource/Image/Cave_Ramp.png"; // 洞窟ステージの三角台足場画像
	static const std::string ClearRope = "Resource/Image/Rope.png"; // 洞窟ステージクリア時の脱出ロープ画像

	// スライム画像
	static const std::string Slime = "Resource/Enemy/Slime/slime_idle.png"; // スライム画像
	static const std::string SlimeIdle = "Resource/Enemy/Slime/slime_idle.png";
	static const std::string SlimeMove = "Resource/Enemy/Slime/slime_move.png";
	static const std::string SlimeAttack = "Resource/Enemy/Slime/slime_attack.png";
	static const std::string SlimeDeath = "Resource/Enemy/Slime/slime_death.png";

	// キングスライム画像
	static const std::string KingSlimeIdle = "Resource/Enemy/KingSlime/kingSlime_idle.png";
	static const std::string KingSlimeMove = "Resource/Enemy/KingSlime/kingSlime_move.png";
	static const std::string KingSlimeJump = "Resource/Enemy/KingSlime/kingSlime_jump.png";
	static const std::string KingSlimeAttack = "Resource/Enemy/KingSlime/kingSlime_attack.png";
	static const std::string KingSlimeLongRangeAttack = "Resource/Enemy/KingSlime/kingSlime_long_range_attack.png";
	static const std::string KingSlimeDeath = "Resource/Enemy/KingSlime/kingSlime_death.png";


	// スケルトン画像
	static const std::string SkeletonIdel = "Resource/Enemy/KingSlime/kingSlime_death.png";
	static const std::string SkeletonDeath = "Resource/Enemy/skeleton_animation/skeleton_animation/skeleton_death.png";


	// 天使の画像
	static const std::string SaintClosed = "Resource/Image/saint_close1.png";
	static const std::string SaintClosedAndOpen = "Resource/Image/saint_close2.png";
	static const std::string SaintOpen = "Resource/Image/saint_open2.png";

	// ギミックの画像
	static const std::string HiddenBlock = "Resource/Image/Block.png";
	static const std::string MoveFloor = "Resource/Image/MoveFloor.png";
	static const std::string MoveFloorBig = "Resource/Image/MoveFloorBig.png";
	static const std::string FeintFloor = "Resource/Image/FeintFloor.png";
	static const std::string CaveFloor = "Resource/Image/CaveFloor.png";

	// ハートの画像
	static const std::string HeartFull = "Resource/Image/heart_full.png";
	static const std::string HeartHalf = "Resource/Image/heart_half.png";
	static const std::string HeartEmpty = "Resource/Image/heart_empty.png";


}

// 背景画像の読み込み
namespace BackGroundGraphPath
{
	// 選択画面の背景
	static const std::string SelectStageMap = "Resource/Backgrounds/selectMap.png";

	// タイトルロゴの背景
	static const std::string TitleLogoGraphs = "Resource/LogoImage/title_logo.png";
	// タイトル雲
	static const std::string TitleScrollCloud = "Resource/SceneBackground/cloud_seamless.png";
	// タイトル背景画像
	static const std::string TitleBackground = "Resource/SceneBackground/background_noclouds.png";
}


// ブロックマップ関係
namespace BlockMapGraphPath
{

	// 1-1ステージ　始まりの森
	static const char* ForestMap = "Resource/Map/ForestMap.png";
	static const char* ForestMapCollision = "Resource/Map/ForestMapCollision.png";

	// 1-2ステージ 洞窟
	static const char* CaveMap = "Resource/Map/CaveMap.png";
	static const char* CaveMapCollision = "Resource/Map/CaveMapCollision.png";

	// 1-2ステージ 洞窟オープニング
	static const char* CaveOpening = "Resource/Map/Cave_Opening.png";
	static const char* CaveOpeningCollision = "Resource/Map/Cave_OpeningCollision.png";
}

// スクロール背景のパス
namespace ScrollGraphPath
{
	static const char* StageForest = "Resource/Backgrounds/stage1-1_background.png";
	static const char* StageCave = "Resource/Backgrounds/stage1-2_background.png";
}

// 読み込むCSVパス
namespace CsvPath
{
	static const std::string TutorialText = "Resource/TutorialTexts/tutorialTextData.csv"; // 始まりの森で読み込むテキスト
	static const std::string TutorialDeathText = "Resource/TutorialTexts/tutorialDeathTextData.csv"; // 死亡テキストのCSV
	static const std::string TutorialCave = "Resource/TutorialTexts/tutorialCaveText.csv"; //洞窟ステージで読み込むテキスト
	static const std::string TutorialCaveOpening = "Resource/TutorialTexts/tutorialCaveOpeningText.csv"; // 洞窟オープニング（穴に落ちる前）で読み込むテキスト
}

// よく使う色
namespace ColorOption
{
	static const int Red = GetColor(240, 10, 10);
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

	// オレンジ 破壊可能
	constexpr int BREAKABLE_R = 255;
	constexpr int BREAKABLE_G = 128;
	constexpr int BREAKABLE_B = 0;
}

// プレイヤーがブロックマップとの当たり判定で使用するサイズ
namespace PlayerBlockCollision
{
	// 壁判定時に床や天井のブロックを誤検出しないように上下を縮める
	static constexpr float WallCheckMargin = 20.0f;
	// 壁や床、天井の判定を行う際の厚み
	static constexpr float CollisionThickness = 1.0f;
	// 坂道・階段状コリジョンを滑らかに移動するための1回あたりの最大水平分割幅(px)
	static constexpr float SlopeSubstepSize = 1.0f;
	// 1pxの水平移動あたり自動で上り上げ可能な坂道の最大段差(px)
	static constexpr int SlopeMaxStepUpPerPixel = 8;
	// 接地中に1pxの水平移動あたり自動で足元を吸着させる下り坂の最大段差(px)
	static constexpr int SlopeMaxStepDownPerPixel = 8;
	// 坂道勾配測定に用いる左右のサンプル半幅(px)
	static constexpr int SlopeMeasureHalfSpan = 8;
	// 坂道高さを探索する上下プローブ範囲(px)
	static constexpr int SlopeProbeRangeY = 24;
	// 勾配がこの上下差(px)以下なら平地とみなして速度補正をかけない
	static constexpr int SlopeFlatIgnoreDeltaY = 3;
	// 坂道段差による描画・カメラY座標のガタつきを抑える平滑化半径(px)
	static constexpr int SlopeVisualSmoothRadius = 6;
	// 坂道から空中へ出た際の描画補正オフセット減衰率
	static constexpr float SlopeAirOffsetDecay = 0.7f;
}

// 表示するテキストの位置
namespace TextPosition
{
	// 落ちた時のGAMEOVER文字
	static const int FallDeathX = 600;
	static const int FallDeathY = ScreenSize::CenterY + 200;

	// 残機表示
	static const int LivesX = ScreenSize::CenterX - 50;
	static const int LivesY = FallDeathY + 200;
}

// チュートリアルテキストの表示関係
namespace TutorialTextControll
{
	static const int TextX = UIGraphController::SaintFramePosX + 280;
	static const int TextY = UIGraphController::FramePosY + 25;
	static const int TextSize = 42;
	static const int DeathTextSize = 55;

	static constexpr float MaxFallDeathTimer = 180.0f;
}

// マップスクロール関係
namespace MapScrollConstants
{
	// プレイヤーがこの画面X座標より右に行ったらスクロール
	static constexpr int ScrollStartRightX = 1000;
	// プレイヤーがこの画面X座標より左に行ったらスクロール
	static constexpr int ScrollStartLeftX = 600;
	// プレイヤーがこの画像Y座標より上下にいったらスクロール
	static constexpr int ScrollStartUpY = 600;

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
	static constexpr float RideMarginTop = 10.0f;    // 上側の許容範囲
	static constexpr float RideMarginBottom = 30.0f; // 下側の許容範囲

	// 乗ったら速度が上がる罠で上昇する速度の倍率
	static constexpr float SpeedUpMultiplier = 3.0f;

	// 一定方向に移動し続けるタイプで画面外と判定して破棄するまでの距離
	static constexpr float DestroyMarginY = 200.0f;

	// 
	static constexpr float ReverseDashMultiplier = 10.0f;
	static constexpr float SpawnYTop = 870.0f;
	static constexpr float SpawnYBottom = 2250.0f;
	static constexpr int   ShrinkDurationFrames = 30; // 指定座標到達時の縮小消滅にかけるフレーム数
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

// 崩落床（クランブルフロア）用の定数
namespace CrumbleFloorConstants
{
	static constexpr float FallGravity       = 1.0f;   // 落下加速度
	static constexpr float MaxFallSpeed      = 28.0f;  // 最大落下速度
	static constexpr float DestroyDistanceY  = 1000.0f; // 初期位置から描画を停止するまでの落下距離
	static constexpr float CollisionTopOffset = 30.0f;  // 当たり判定の上面削り量
}

// スライム用の定数
namespace SlimeConstants
{
	static constexpr float CollisionWidth = 70.0f;  // 当たり判定の幅
	static constexpr float CollisionHeight = 65.0f; // 当たり判定の縦幅
	static constexpr int   TackleChargeFrames = 30; // 体当たり攻撃の起こり（溜め）フレーム数
	static constexpr float TackleShakePower   = 2.2f; // 起こり（溜め）中の震えの強さ
}

// スライム死亡演出用の定数
namespace SlimeDeathConstants
{
	static constexpr int   AnimTotalFrames = 5;    // 死亡アニメーション総コマ数
	static constexpr int   AnimInterval    = 8;    // コマ送り間隔
	static constexpr int   FadeOutDuration = 60;   // 消滅時間
	static constexpr float SinkSpeed       = 0.5f; // 沈み込む速度
}

// スケルトン死亡演出用の定数
namespace SkeletonDeathConstants
{
	static constexpr int AnimTotalFrames      = 8;   // skeleton_death.png の総コマ数
	static constexpr int FrameWidth           = 300; // 1コマ横幅(px)
	static constexpr int FrameHeight          = 300; // 1コマ縦幅(px)
	static constexpr int AnimInterval         = 6;   // 死亡アニメのコマ送り間隔(frame)
	static constexpr int CorpseFadeFrameIndex = 5;   // 最終コマ再生後に切り替える6フレーム目(0始まり)
	static constexpr int FadeOutDuration      = 60;  // 6フレーム目表示中のフェードアウト時間(frame)
}

// ボス戦開始演出用の定数
namespace BossIntroConstants
{
	static constexpr int LetterboxHeight = 50;   // 黒帯の最大高さ
	static constexpr int LetterboxAlpha  = 255;  // 黒帯の最大不透明度
	static constexpr int FadeFrames      = 60;   // 黒帯フェードインのフレーム数
	static constexpr float AutoWalkSpeed = 3.0f; // 演出中の自動歩行速度
}

// キングスライム出現演出・警告用定数
namespace KingSlimeIntroConstants
{
	static constexpr float Scale                 = 4.0f;  // キングスライムの拡大率
	static constexpr float HitBoxScale			 = 3.0f;  // キングスライムの当たり判定の拡大率
	static constexpr int WaitFramesBeforeWarning = 45;    // 到着後の待機フレーム
	static constexpr int WarningDurationFrames   = 75;    // 警告表示フレーム
	static constexpr int WarningFadeOutFrames    = 15;    // 警告フェードアウトフレーム
	static constexpr float WarningWidth          = 80.0f * Scale; // 警告の横幅
	static constexpr float WarningHeight         = 450.0f;// 警告の縦幅
	static constexpr float EvasionSpeedX         = 8.0f; // 回避ジャンプの水平移動速度
	static constexpr float EvasionJumpSpeedY     = -10.0f;// 回避ジャンプの初速Y
	static constexpr int EvasionDurationFrames   = 22;    // 回避ジャンプの持続フレーム数
}

// キングスライム着地時の画面揺れ用定数
namespace KingSlimeShakeConstants
{
	static constexpr float ShakePower     = 30.0f; // 揺れの最大強度
	static constexpr float RatioY         = 0.8f;  // 縦方向の揺れ割合
	static constexpr float RatioX         = 0.2f;  // 横方向の揺れ割合
	static constexpr int   DurationFrames = 30;    // 揺らすフレーム数
}

// キングスライム登場演出のタイトル表示設定
namespace KingSlimeTitleConstants
{
	static constexpr int   WaitFramesBeforeTitle  = 20;     // 画面揺れ収束後の待機フレーム
	static constexpr int   CharRevealInterval     = 6;      // 1文字ごとの出現間隔フレーム
	static constexpr int   HoldFramesAfterReveal  = 90;     // 全文字出現後の表示維持フレーム
	static constexpr int   FadeOutFrames          = 25;     // 文字消滅のフェードアウトフレーム
	static constexpr int   FontSize               = 84;     // タイトル文字のフォントサイズ
	static constexpr int   CharSpacing            = 4;      // 文字間スペース
	static constexpr float WaveAmplitude          = 8.0f;   // 縦揺れの振幅
	static constexpr float WaveSpeed              = 0.08f;  // 縦揺れの周期速度
	static constexpr float WavePhaseOffset        = 0.45f;  // 隣接文字間の波の位相差
	static constexpr int   ColorGoldR             = 255;    // 金色
	static constexpr int   ColorGoldG             = 215;    
	static constexpr int   ColorGoldB             = 0;      
	static constexpr int   ColorCyanR             = 30;     // 濃い水色
	static constexpr int   ColorCyanG             = 160;    
	static constexpr int   ColorCyanB             = 240;    
	static constexpr int   CharCount              = 8;      // タイトル文字数「キングスライミー」
}

// キングスライムAI（ジャンプ攻撃・ビーム攻撃）用の定数
namespace KingSlimeAIConstants
{
	// 待機・ターゲット選択関連
	static constexpr int   IdleWaitFrames            = 60;     
	static constexpr float EdgeOffsetX               = 650.0f; // ボスエリア中央から左右端へのオフセット距離
	static constexpr float CenterZoneHalfWidth       = 240.0f; // プレイヤーが中央付近にいると判定する半幅
	static constexpr int   RandomTargetChancePercent = 20;     // ロジック無視確率

	// ジャンプ攻撃関連
	static constexpr int   JumpPrepareAnimInterval   = 4;      
	static constexpr float JumpAscendSpeed           = 34.0f;  // 上空へ飛び上がる速度
	static constexpr float JumpFallSpeed             = 26.0f;  // 急降下速度
	static constexpr float JumpStartHeightOffset     = 1200.0f;// 落下開始高度
	static constexpr int   JumpWarningDurationFrames = 50;     
	static constexpr int   JumpWarningFadeOutFrames  = 15;     

	// ビーム攻撃関連
	static constexpr int   BeamMouthOpenInterval     = 5;      // 1〜3枚目の再生間隔
	static constexpr int   BeamChargeFrames          = 80;     
	static constexpr int   BeamConvergeFrames        = 36;     
	static constexpr int   BeamFireStartInterval     = 4;      
	static constexpr int   BeamFireHoldFrames        = 65;     
	static constexpr int   BeamRecoveryInterval      = 6;      // 12〜13枚目の再生間隔

	// ビーム座標・サイズ・演出関連
	static constexpr float BeamMouthOffsetX          = 126.0f; 
	static constexpr float BeamMouthOffsetY          = 84.0f;  
	static constexpr float BeamLength                = 1800.0f;// レーザーの長さ
	static constexpr float BeamHalfHeight            = 48.0f;  // レーザーの当たり判定・描画の半分の高さ
	static constexpr float BeamOrbMaxRadius          = 44.0f;  
	static constexpr float BeamOrbMinRadius          = 8.0f;   
	static constexpr int   BeamParticleCount         = 24;     
	static constexpr int   BeamDotPixelSize          = 4;      

	// 当たり判定・見た目調整関連
	static constexpr float HitBoxBottomExtend        = 36.0f;  // 当たり判定下部をさらに下へ伸ばす拡張量

	// 体当たり攻撃関連
	static constexpr float TackleRangeMax            = 650.0f; // 判定距離
	static constexpr int   TackleChancePercent       = 40;     // 発動確率
	static constexpr int   TackleCooldownFrames      = 180;    // クールタイム
	static constexpr int   TackleAnimInterval        = 8;      
	static constexpr float TackleJumpSpeedY          = -8.0f;  
	static constexpr float TackleGravity             = 0.5f;   // 体当たり時の重力
	static constexpr float TackleOvershootX          = 5.0f; // プレイヤーの目標座標を通り越す追加距離
	static constexpr int   TackleChargeFrames        = 36;     
	static constexpr float TackleShakePower          = 4.5f;   

	// 着地時スライム液体弾関連
	static constexpr int   DropletCount              = 10;     // 個数
	static constexpr float DropletSpeedMin           = 10.0f;  // 弾の初速
	static constexpr float DropletSpeedMax           = 16.0f; 
	static constexpr float DropletGravity            = 0.28f;  // 弾の重力
	static constexpr float DropletRadiusMin          = 10.0f;  // 弾の半径
	static constexpr float DropletRadiusMax          = 16.0f;  
	static constexpr int   DropletLifeFrames         = 180;    // 弾の最大生存フレーム数
	static constexpr int   DropletSplashFrames       = 10;     // 床着地時の演出フレーム数
	static constexpr float DropletAngleRangeDeg      = 45.0f;  
	static constexpr float DropletSpawnSpreadX       = 35.0f;  
	static constexpr float DropletSpawnOffsetY       = -75.0f; 
	static constexpr float DropletHpThresholdRatio   = 0.5f;   // 液体弾が発動するHP割合
}

// キングスライム地面衝撃波用の定数（反比例型＋上面ギザギザ波）
namespace KingSlimeShockwaveConstants
{
	// 大ジャンプ必須の衝撃波（プレイヤー全高100px・中ジャンプ207pxを超える高さ、大ジャンプ334pxで回避可能）
	static constexpr float Height          = 195.0f; // 先端最大高さ
	static constexpr float Width           = 55.0f;  // 当たり判定幅（先端主要部）
	static constexpr float Speed           = 6.0f;   // 地面を這う速度（px/frame）
	static constexpr float VisualLength    = 135.0f; // 描画用の全長（後ろへの尾引き幅）
	static constexpr int   SpikeCount      = 5;      // 上面ギザギザ（鋸歯スパイク）の数

	static constexpr int   LifeFrames      = 240;    // 安全上の最大生存フレーム数
	static constexpr float WallLimitX      = 650.0f; // 中央からの消滅開始指定座標までの距離
	static constexpr int   Damage          = 1;      // ダメージ量
}

// キングスライムHPバー用定数
namespace KingSlimeHpBarConstants
{
	static constexpr int   Width                = 800;   // HPバーの全幅
	static constexpr int   Height               = 24;    // HPバーの高さ
	static constexpr int   PosY                 = 980;   // HPバーのY座標
	static constexpr int   BorderThickness      = 3;     // 白枠の太さ
	static constexpr int   Padding              = 2;     // 枠線と赤いバーの内側余白
	static constexpr int   YellowDelayFrames    = 25;    
	static constexpr float YellowDrainSpeed     = 0.6f;  
}
// キングスライム死亡演出用定数
namespace KingSlimeDeathConstants
{
	static constexpr int   AnimInterval          = 8;      
	static constexpr int   PostAnimPauseFrames   = 25;     // 最終コマ停止時間
	static constexpr int   ShakeDurationFrames   = 45;     // 左右小刻み震え継続時間
	static constexpr float ShakePower            = 6.0f;  
	static constexpr int   ShakeInterval         = 3;      
	static constexpr int   PreExplodePauseFrames = 10;     // 震え停止後〜爆発までの時間
	static constexpr int   DeathDropletCount     = 24;     // 個数
	static constexpr float DeathDropletSpeedMin  = 10.0f;  
	static constexpr float DeathDropletSpeedMax  = 18.0f;  
	static constexpr int   PostExplosionWaitFrames = 180;  
}

// ステージクリア演出用定数
namespace StageClearConstants
{
	static constexpr int   StartDelayFrames      = 15;     // 開始までの待機フレーム数
	static constexpr int   CrackerSlideFrames    = 22;     // スライドイン所要フレーム数
	static constexpr int   CrackerAimWaitFrames  = 60;     // 停止後、発射までの待機フレーム数
	static constexpr int   AimLockFramesBeforeBurst = 18;  // 発射何フレーム前に照準を固定するか
	static constexpr float CrackerScale          = 3.8f;   // 画像描画スケール
	static constexpr float CrackerLeftStartX     = -160.0f;
	static constexpr float CrackerLeftTargetX    = 90.0f;  
	static constexpr float CrackerRightStartX    = 2080.0f;
	static constexpr float CrackerRightTargetX   = 1830.0f;
	static constexpr float CrackerPosYTop        = 320.0f; 
	static constexpr float CrackerPosYMid        = 540.0f; 
	static constexpr float CrackerPosYBottom     = 760.0f; 
	static constexpr int   ConfettiPerCracker    = 25;     // 紙吹雪個数
	static constexpr int   ConfettiLifeFrames    = 180;    // 紙吹雪滞空寿命

	// マズルフラッシュ用定数
	static constexpr int   FlashFrameCount       = 5;      // アニメーション総コマ数
	static constexpr int   FlashFrameWidth       = 48;     // コマ横幅
	static constexpr int   FlashFrameHeight      = 48;     // コマ縦幅
	static constexpr int   FlashAnimInterval     = 2;      // 各コマの表示フレーム数（計10フレーム）
	static constexpr float FlashScale            = 3.8f;   // 描画スケール（クラッカー等身と統一）

	// クラッカー弾用定数
	static constexpr int   BulletsPerCracker     = 3;      // 弾丸発射数
	static constexpr float BulletRadius          = 12.0f; // 弾の半径
	static constexpr float BulletAimedSpeed      = 22.0f;  // 狙撃弾の初速
	static constexpr float BulletAimedGravity    = 0.10f;  // 狙撃弾の重力
	static constexpr float BulletSkySpeed        = 24.0f;  // 空中打ち上げ弾の初速
	static constexpr float BulletSkyGravity      = 0.45f;  // 空中打ち上げ弾の重力
	static constexpr int   BulletDamagePostTutorial = 10;  // 初期以降の弾丸ダメージ

	// タイトルロゴ落下演出用定数
	static constexpr float LogoInitialY          = -650.0f; 
	static constexpr float LogoFallGravity       = 2.4f;    // 重力加速度
	static constexpr float LogoFallMaxSpeed      = 54.0f;   // 最高速度
	static constexpr float LogoTiltAngleRad      = -0.2618f;// 傾き
	static constexpr float LogoGroundY           = 709.0f;  
	static constexpr float LogoEmbedDepth        = 35.0f;   
	static constexpr float LogoShakePower        = 22.0f;   
	static constexpr int   LogoShakeDuration     = 30;      
	static constexpr int   LogoPostImpactWaitFrames = 30;   
	static constexpr int   CrackerBurstMaxFrames = 240;     
	static constexpr int   PostBulletWaitFrames  = 35;      
	static constexpr float LogoPreFallShakePower = 16.0f;   // ロゴ落下直前の横揺れ
	static constexpr int   LogoPreFallShakeDuration = 30;   
	static constexpr int   LogoPreFallShakeFrames = 30;     

	// タイトルロゴ当たり判定サイズ
	static constexpr float LogoHitboxHalfWidth   = 240.0f;  // 当たり判定
	static constexpr float LogoHitboxHalfHeight  = 160.0f;  

	// クリアテキスト表示用定数
	static constexpr int   FontSize              = 120;    // フォントサイズ
	static constexpr int   OutlineWidth          = 4;      // 黒枠アウトライン太さ
	static constexpr int   FixedPosY             = 360;    
	static constexpr int   BlinkDurationFrames   = 50;     // 点滅表示時間
	static constexpr int   BlinkIntervalFrames   = 5;      // 点滅切り替え間隔
	static constexpr int   DisplayDurationFrames = 240;

	// 演出終了後のフェード・扉出現待機定数
	static constexpr int   ClearCleanUpFrames    = 30;     // クラッカー退場・ロゴ・文字フェード時間
	static constexpr int   DoorAppearWaitFrames  = 35;     
	static constexpr int   DoorAppearFrames      = 20;     // 扉出現ポップ時間

	// クリア扉用定数
	static constexpr int   DoorFrameCount        = 4;      // アニメーションコマ数
	static constexpr int   DoorFrameWidth        = 128;    
	static constexpr int   DoorFrameHeight       = 128;    
	static constexpr float DoorScale             = 1.0f;   
	static constexpr float DoorSpawnScreenX      = 960.0f; 
	static constexpr float DoorInteractRangeX    = 120.0f; 
	static constexpr float DoorInteractRangeY    = 150.0f; 
	static constexpr float DoorAnimSpeed         = 0.15f;  // アニメーション再生速度
	static constexpr float DoorGroundFallbackY   = 770.0f;    

	// 洞窟ステージ クリアロープ演出用定数
	static constexpr int   RopeTexWidth              = 20;     // Rope.png 画像幅(px)
	static constexpr int   RopeSegmentHeight         = 42;     // 結び目1周期分の切り出し高さ(px)
	static constexpr float RopeScale                 = 1.5f;   // 描画倍率（マップ・プレイヤーの3pxドット密度に統一）
	static constexpr float RopeGroundClearanceY      = 45.0f;  // 地面からロープ下端までの隙間(px)
	static constexpr float RopeDropStartOffsetY      = 180.0f; // 天井穴下端から上方に引いた落下開始オフセット(px)
	static constexpr int   RopeAppearFrames          = 38;     // ロープが穴から垂れ切るまでのフレーム数
	static constexpr float RopeDropOvershootY        = 18.0f;  // 停止時の下方向へのバウンド伸び量(px)
	static constexpr float RopeInteractRangeX        = 75.0f;  // ロープを掴める横幅許容距離(px)
	static constexpr float RopeClimbSpeedY           = 6.5f;   // ロープ登り上昇速度(px/frame)
	static constexpr float RopeClimbWobbleX          = 2.5f;   // 登り中の左右の体の揺れ幅(px)
	static constexpr int   RopeClimbAnimFlipInterval = 10;     // 手足交互登り風の向き反転間隔(frame)
	static constexpr int   RopeClimbFadeStartYOffset = 140;    // 画面上端付近で白フェードを開始する余白(px)
	static constexpr int   RopeClimbExitYOffset      = 50;     // 画面上端を完全に抜けてクリア遷移する余白(px)
}

// GameLoadData用定数
namespace GameLoadDataConstants
{
	// ステージ識別番号
	static constexpr int StageIndexForest = 0;
	static constexpr int StageIndexCave = 1;

	// --- プレイヤー初期座標 ---
	static constexpr float ForestPlayerSpawnX = 100.0f;
	static constexpr float ForestPlayerSpawnY = 1485.0f;

	static constexpr float CavePlayerSpawnX = 100.0f;
	static constexpr float CavePlayerSpawnY = 979.0f;

	// --- 始まりの森（StageForest）配置定数 ---
	// フェイント床（FeintFloor）
	static constexpr float ForestFeintFloorPosX = 4920.0f;
	static constexpr float ForestFeintFloorPosY = 1685.0f;

	// 崩れる床（CrumbleFloor）
	static constexpr float ForestCrumbleFloorPosX = 7599.0f;
	static constexpr float ForestCrumbleFloorPosY = 1600.0f;

	// 隠しブロック（HiddenBlock）
	static constexpr float ForestHiddenBlockPosX = 3150.0f;
	static constexpr float ForestHiddenBlockPosY = 1175.0f;

	// 動く足場スポナー共通
	static constexpr int ForestSpawnerInterval = 120; // 生成インターバル（フレーム）

	// 動く足場スポナー1（下から上へ移動：通常）
	static constexpr float ForestSpawner1PosX = 6300.0f;
	static constexpr float ForestSpawner1PosY = 2250.0f;
	static constexpr float ForestSpawner1VelY = -3.0f;

	// 動く足場スポナー2（上から下へ移動：乗ると加速）
	static constexpr float ForestSpawner2PosX = 6575.0f;
	static constexpr float ForestSpawner2PosY = 870.0f;
	static constexpr float ForestSpawner2VelY = 3.0f;

	// 動く足場スポナー3（下から上へ移動：反転＆ダッシュ）
	static constexpr float ForestSpawner3PosX = 6850.0f;
	static constexpr float ForestSpawner3PosY = 2250.0f;
	static constexpr float ForestSpawner3VelY = -3.0f;

	// スライム（EnemySlime）
	static constexpr float ForestSlime1PosX = 1000.0f;
	static constexpr float ForestSlime1PosY = 300.0f;

	static constexpr float ForestSlime2PosX = 4000.0f;
	static constexpr float ForestSlime2PosY = 1500.0f;

	static constexpr float ForestNeedle1PosX = 1404.0f;
	static constexpr float ForestNeedle1PosY = 1490.0f;

	static constexpr float ForestNeedle2PosX = 1500.0f;
	static constexpr float ForestNeedle2PosY = 1490.0f;

	static constexpr float ForestNeedle3PosX = 1596.0f;
	static constexpr float ForestNeedle3PosY = 1490.0f;

	static constexpr float ForestNeedle4PosX = 4280.0f;
	static constexpr float ForestNeedle4PosY = 1600.0f;

	static constexpr float ForestNeedle5PosX = 4376.0f;
	static constexpr float ForestNeedle5PosY = 1600.0f;

	static constexpr float ForestNeedle6PosX = 5980.0f;
	static constexpr float ForestNeedle6PosY = 1710.0f;

	// --- 洞窟ステージ（StageCave）配置定数 ---
	// スケルトン（EnemySkeleton）
	static constexpr float CaveSkeleton1PosX = 1000.0f;
	static constexpr float CaveSkeleton1PosY = 976.0f;

	static constexpr float CaveSkeleton2PosX = 2400.0f;
	static constexpr float CaveSkeleton2PosY = 976.0f;

	static constexpr float CaveSkeleton3PosX = 6745.0f;
	static constexpr float CaveSkeleton3PosY = 1850.0f;

	// 往復動く足場（MovingFloor）
	static constexpr float CaveMovingFloorStartX = 7000.0f;
	static constexpr float CaveMovingFloorEndX = 7550.0f;
	static constexpr float CaveMovingFloorPosY = 1800.0f;
	static constexpr float CaveMovingFloorSpeed = 2.0f;
	static constexpr float CaveMovingFloorScale = 0.40f;

	// 転がり岩の坂下くぼみ巨大針（BoulderNeedleTrap）
	static constexpr float CaveBoulderSpikePosX = 4315.5f;
	static constexpr float CaveBoulderSpikePosY = 2006.0f;

	// 凹みに敷き詰める針（NeedleTrap: BloodNeedle）
	static constexpr float CavePitStartX = 6940.0f;
	static constexpr float CavePitNeedleWidth = 72.0f;
	static constexpr float CavePitNeedleCenterY = 2052.0f;
	static constexpr float CavePitNeedleScale = 1.0f;
	static constexpr int   CavePitNeedleCount = 10;
	static constexpr int   CavePitDummyNeedleIndex = 3;     // 左から4本目（0-basedで3）のダミー針
	static constexpr float DummyNeedleDetectDistX = 600.0f; // プレイヤー接近感知距離
	static constexpr int   DummyNeedleShakePowerX = 3;      // 左右の震え幅（±3px）
	static constexpr int   DummyNeedleShakePowerY = 1;      // 上下の震え幅（±1px）

	// 壊せる壁前の落下針（NeedleTrap: Falling、先ほど発射された針が落ちてくる設定）
	static constexpr float CaveFallingNeedlePosX = 7780.0f;
	static constexpr float CaveFallingNeedleInitialY = 1000.0f;
	static constexpr float CaveFallingNeedleFloorY = 1903.0f;
	static constexpr float CaveFallingNeedleTriggerDistX = 90.0f;
	static constexpr float CaveFallingNeedleFallSpeed = 38.0f; // 落下速度（元30.0fから少し上昇）

	// --- デバッグ表示用座標 ---
	static constexpr int DebugProgressTextX = 10;
	static constexpr int DebugProgressTextY = 30;
	static constexpr int DebugWarpTextX = 10;
	static constexpr int DebugWarpTextY = 50;
}

// デバッグワープ機能用定数
namespace DebugWarpConstants
{
	// 森ステージ（キングスライム戦前）の安全な足場
	static constexpr float ForestBossPreWarpX = 7660.0f; // ボスエリア手前の高台足場中央
	static constexpr float ForestBossPreWarpY = 1404.0f; // ブロック上面Y=1454に対するプレイヤー中心Y

	// 洞窟ステージ（ドラゴン戦前）の安全な足場
	static constexpr float CaveBossPreWarpX   = 7935.0f; // 壊せる壁通過後、ボスエリア直前の通路中央
	static constexpr float CaveBossPreWarpY   = 1853.0f; // ブロック上面Y=1903に対するプレイヤー中心Y

	// ガイドテキスト表示位置
	static constexpr int GuideTextX = 10;
	static constexpr int GuideTextY = 50;
}

// トロッコ（Minecart）関連定数
namespace MinecartConstants
{
	// 三角台スプライトの位置・サイズ
	static constexpr int RampSpriteX = 5359;
	static constexpr int RampSpriteY = 1800;
	static constexpr int RampSpriteWidth = 96;
	static constexpr int RampSpriteHeight = 96;

	// 三角台コリジョン範囲
	static constexpr int RampStartX = 5359;
	static constexpr int RampEndX = 5440;
	static constexpr int RampTopY = 1800;
	static constexpr int RampFloorY = 1896;
	static constexpr int RampHeight = RampFloorY - RampTopY; // 96

	// コリジョン消去範囲（左端の1px壁や余白を確実に巻き込むためX=5350から消去）
	static constexpr int RampClearStartX = 5350;
	static constexpr int RampClearWidth = RampEndX - RampClearStartX + 1; // 91

	// 三角台破壊時のカメラシェイク
	static constexpr float SmashShakePower = 7.0f;
	static constexpr int SmashShakeFrames = 12;

	// トロッコのリセット判定（プレイヤーが初期位置付近に戻った時）
	static constexpr float PlayerRespawnThresholdX = 500.0f;
}

// 温泉（HotSpring）関連定数
namespace HotSpringConstants
{
	// 洞窟ステージの温泉配置範囲（くぼみの地形に隙間なく合致する座標）
	static constexpr float LeftX = 6185.0f;       // くぼみ左端（左壁 X=6185/6186）
	static constexpr float RightX = 6508.0f;      // くぼみ右端（右壁 X=6508/6509）
	static constexpr float BaseWaterY = 1905.0f;  // 水面基準高さ
	static constexpr float FloorY = 2006.0f;      // くぼみ底面（マップ画像およびコリジョンの地面上面 Y=2006）
}

// 洞窟ボス（ドラゴン）登場時の天井破砕演出用定数
namespace DragonCeilingBreakConstants
{
	static constexpr int   ChunkSize              = 24;      // 天井を砕くブロック1片のサイズ(px)
	static constexpr int   Cols                   = 16;      // くり抜き領域の横ブロック数（16 * 24 = 384px）
	static constexpr int   Rows                   = 19;      // くり抜き領域の縦ブロック数（19 * 24 = 456px）
	static constexpr int   CeilingBottomY         = 1286;    // 洞窟天井の下端Y座標
	static constexpr int   CeilingTopY            = CeilingBottomY - (Rows * ChunkSize); // 830
	static constexpr float FloorY                 = 1903.0f; // ボスエリア地面Y座標

	static constexpr float BreakFrontOffsetY      = 48.0f;   // ドラゴン先端の通過判定先行マージン（砕けながら先行落下させる）
	static constexpr float ShakePreviewDistY      = 90.0f;   // 破壊直前に天井ブロックが振動する距離
	static constexpr float VisibleCeilingTriggerY = 1085.0f; // 画面内天井へ突入した瞬間のSE・カメラシェイク発動Y
	static constexpr float PenetrateStartY        = 1040.0f; // 天井抵抗で減速しながら砕き始めるY
	static constexpr float PenetrateEndY          = 1300.0f; // 天井突破完了して再加速するY
	static constexpr float PenetrateSpeedY        = 16.0f;   // 天井を砕きながら落ちる際の重厚な落下速度
	static constexpr float NormalDropSpeedY       = 30.0f;   // 天井突破前後の通常急降下速度
	static constexpr float ChunkGravity           = 0.65f;   // 落下する天井破片の重力加速度
	static constexpr float BreakShakePower        = 14.0f;   // 天井貫通時の画面揺れ強度
	static constexpr int   BreakShakeFrames       = 22;      // 天井貫通時の画面揺れフレーム数
}

// 洞窟オープニング（Cave_Opening）用定数
namespace CaveOpeningConstants
{
	// オープニング開始座標（床面Y=828に足元接地）
	static constexpr float PlayerSpawnX = 100.0f;
	static constexpr float PlayerSpawnY = 778.0f;

	// 通路上下範囲と崩落穴（オレンジ色領域）のデフォルト座標
	static constexpr int CorridorTopY    = 324;
	static constexpr int CorridorBottomY = 827;
	static constexpr int PitLeftX        = 2999;
	static constexpr int PitRightX       = 3822;
	static constexpr int PitTopY         = 828;
	static constexpr int PitBottomY      = 1367;

	// 右端から洞窟外へ続くように見せる光の演出定数
	static constexpr float ExitLightWidth     = 720.0f; // 右端から洞窟内へ差し込む光の到達幅(px)
	static constexpr float ExitLightCoreWidth = 110.0f; // 出口境界付近の最も強い光の幅(px)
	static constexpr int   ExitLightRayCount  = 3;      // 斜めに差し込む光条の数
	static constexpr int   ExitLightDustCount = 22;     // 光の中を漂う塵パーティクル数

	// 床崩落破片エフェクト定数
	static constexpr int   PitChunkSize        = 48;    // 床破片チャンクの1辺サイズ(px)
	static constexpr float PitChunkGravity     = 0.72f; // 落下破片の重力加速度
	static constexpr float PitBreakShakePower  = 9.0f;  // 床崩壊時の画面揺れ強度
	static constexpr int   PitBreakShakeFrames = 18;    // 床崩壊時の画面揺れフレーム数

	// 穴落下後から洞窟本編（CaveMap）への遷移・落下登場定数
	static constexpr float TransitionFadeStartY    = 1160.0f; // 暗転フェード開始する穴内Y座標
	static constexpr int   TransitionFadeOutFrames = 14;      // 暗転フェードアウト所要フレーム数
	static constexpr int   TransitionFadeInFrames  = 18;      // 本編登場時フェードイン所要フレーム数
	static constexpr float MainCaveDropStartY      = 415.0f;  // CaveMap開始時の上空落下開始Y（天井Y=360直下）
	static constexpr float MainCaveDropInitialVy   = 10.0f;   // 本編側での最低落下初速Y
	static constexpr float MainCaveDropMaxCarryVy  = 14.0f;   // 引き継ぐ落下速度の上限

	// ボス撃破後ロープ登頂〜右側足場＆再度降りてのクリア扉定数
	static constexpr float PostBossSpawnX               = 3880.0f; // 割れた穴（X=2999〜3822）の右側足場出現X座標
	static constexpr float PostBossScrollUnlockX        = 4020.0f; // 光へ向かって前進した際に右端先の空間が見え始めるX座標
	static constexpr int   PostBossRightScrollExtra     = 360;     // 右端の光方向へ進める追加カメラスクロール幅(px)
	static constexpr float PostBossRightPlayerLimitOffset = 420.0f;// 右端外落下用のプレイヤーX許容追加幅(px)
	static constexpr float BossReturnDropStartY         = 1120.0f; // 割れた穴から再降下した時のボス天井穴Y座標
	static constexpr float BossReturnDoorX              = 9650.0f; // ボスエリア右側に出現するクリア扉ワールドX座標
	static constexpr float BossReturnDoorGroundY        = 1903.0f; // ボスエリア床Y座標
}
