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

	static const int PlayerAttackWidth = 600; // プレイヤーの攻撃範囲の幅
	static const int PlayerAttackHeight = 40; // プレイヤーの攻撃範囲の高さ

	static const int MaxLive = 3; // プレイヤーの最大残機

	// 落下でプレイヤーが死亡する位置
	static const int PlayerDeathHeight = 3000;
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
	static const std::string KingSlimeLongRangeAttack = "Resource/Enemy/KingSlime/kingSlime_long_range_attack.png";
	static const std::string KingSlimeDeath = "Resource/Enemy/KingSlime/kingSlime_death.png";


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
	static const std::string TutorialText = "Resource/TutorialTexts/tutorialTextData.csv"; // チュートリアルテキストのCSV
	static const std::string TutorialDeathText = "Resource/TutorialTexts/tutorialDeathTextData.csv"; // 死亡テキストのCSV
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
	static const int FallDeathX = 600;
	static const int FallDeathY = ScreenSize::CenterY + 200;

	// 残機表示
	static const int LivesX = ScreenSize::CenterX - 50;
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
	static constexpr float CollisionWidth = 70.0f;  // 当たり判定の横幅
	static constexpr float CollisionHeight = 65.0f; // 当たり判定の縦幅
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
	static constexpr int   CharCount              = 8;      // タイトル文字数（「キングスライミー」で8文字）
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

	// 銀色弾（円形弾）用定数
	static constexpr int   BulletsPerCracker     = 3;      // 弾丸発射数
	static constexpr float BulletRadius          = 12.0f; // 弾の半径（視認性向上）
	static constexpr float BulletAimedSpeed      = 22.0f;  // 狙撃弾の初速
	static constexpr float BulletAimedGravity    = 0.10f;  // 狙撃弾の重力
	static constexpr float BulletSkySpeed        = 24.0f;  // 空中打ち上げ弾の初速
	static constexpr float BulletSkyGravity      = 0.45f;  // 空中打ち上げ弾の重力

	// タイトルロゴ落下演出用定数
	static constexpr float LogoInitialY          = -650.0f; // 画面はるか上空の開始Y座標
	static constexpr float LogoFallGravity       = 2.4f;    // 落下重力加速度
	static constexpr float LogoFallMaxSpeed      = 54.0f;   // 最高落下終端速度
	static constexpr float LogoTiltAngleRad      = -0.2618f;// 傾き角度（-15度、左下角が下を向く）
	static constexpr float LogoGroundY           = 709.0f;  // ボスエリア床面Y座標
	static constexpr float LogoEmbedDepth        = 35.0f;   // 地面への突き刺さり深さ
	static constexpr float LogoShakePower        = 22.0f;   // 着地時カメラシェイク強度
	static constexpr int   LogoShakeDuration     = 30;      // 画面揺れ継続フレーム数
	static constexpr int   LogoPostImpactWaitFrames = 30;   // 着地激突後、文字表示開始までの余韻待機時間
	static constexpr int   CrackerBurstWaitFrames = 45;     // クラッカー発射からロゴ落下開始までの時間（約0.75秒）

	static constexpr int   FontSize              = 120;    // フォントサイズ
	static constexpr int   OutlineWidth          = 4;      // 黒枠アウトライン太さ
	static constexpr int   FixedPosY             = 360;    // 固定描画Y座標
	static constexpr int   BlinkDurationFrames   = 50;     // 点滅表示時間
	static constexpr int   BlinkIntervalFrames   = 5;      // 点滅切り替え間隔
	static constexpr int   DisplayDurationFrames = 240;    // クリア表示時間
}


