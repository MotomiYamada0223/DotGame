#pragma once
#include "DxLib.h"
#include <vector>

class BlockMap;

// =========================================================================
// プロジェクト全体の 2D Effekseer エフェクト統合管理・使用箇所カタログ
// どのステージ・どのボス・どの攻撃で使用しているかを一覧で管理する
// =========================================================================
enum class Effect2DStage
{
	Common = 0, // 全ステージ共通
	Stage1,     // ステージ1
	Stage2,     // ステージ2
	Stage3,     // ステージ3（アルティメットドラゴン戦）
	Reserved    // 予備登録（現在未使用）
};

enum class Effect2DId
{
	Stage3_RoarWave = 0, // 咆哮・衝撃波（SonicBoom.efk）
	Stage3_WindTornado,  // 風の渦（ForceFieldTornado.efkefc）
	Stage3_FlameBurst,   // 火炎バースト（tktk01/Sword1.efk）
	Stage3_HolyStorm,    // 嵐・衝撃波（tktk01/Blow5.efk）
	Stage3_Explosion,    // 砂嵐・爆風（Pierre01/HolySandstorm.efk）
	Stage3_Thunder,      // 雷撃・放電ボルト（NextSoft01/Sword2.efk）
	Stage3_SlashWind,    // 風斬撃・引き寄せ風（tktk01/Sword5.efk）
	Stage3_Aura,         // 予備オーラ（SonicBoom.efk）
	Stage3_HitLight,     // 予備ヒット閃光（tktk01/Light1.efk）
	Stage3_MagicCircle,  // 分身魔法陣（NextSoft01/MagicShield.efk）
	Count
};

struct Effect2DInfo
{
	Effect2DId    id;
	const char*   name;       // エフェクト識別名
	const char*   path;       // .efk / .efkefc ファイルパス
	float         baseScale;  // 基準スケール
	Effect2DStage stage;      // 使用ステージ
	const char*   usedWhere;  // 使用箇所の詳細（クラス・攻撃名）
};

class Effekseer2DManager
{
public:
	Effekseer2DManager();
	~Effekseer2DManager();

	void Initialize();
	void Update(const BlockMap& blockMap, int viewWidth, int viewHeight);
	void UpdateWithScroll(float scrollX, float scrollY, int viewWidth, int viewHeight);
	void Draw();
	void StopAll();
	void Finalize();

	int  PlayEffect(Effect2DId id, float worldX, float worldY, float scaleMul = 1.0f, float speed = 1.0f, float rotZ = 0.0f, int r = 255, int g = 255, int b = 255, int a = 255);
	void UpdateEffectTransform(int playingHandle, float worldX, float worldY, float scaleMul = -1.0f, float rotZ = 0.0f);
	void StopEffect(int playingHandle);
	bool IsEffectPlaying(int playingHandle) const;

	// 静的ショートカット API
	static int  Play(Effect2DId id, float worldX, float worldY, float scaleMul = 1.0f, float speed = 1.0f, float rotZ = 0.0f, int r = 255, int g = 255, int b = 255, int a = 255);
	static void SetTransform(int playingHandle, float worldX, float worldY, float scaleMul = -1.0f, float rotZ = 0.0f);
	static void Stop(int playingHandle);
	static bool IsPlaying(int playingHandle);

	// ステージごとの使用箇所情報取得
	static const Effect2DInfo& GetInfo(Effect2DId id);
	static const Effect2DInfo* GetAllInfos(int& outCount);

private:
	struct ActiveEffect2D
	{
		int        playingHandle;
		Effect2DId id;
		float      worldX;
		float      worldY;
		float      baseScale;
	};

	static const int MAX_ACTIVE_EFFECTS = 32;

	int   mResourceHandles[static_cast<int>(Effect2DId::Count)];
	float mBaseScales[static_cast<int>(Effect2DId::Count)];
	std::vector<ActiveEffect2D> mActiveEffects;

	static Effekseer2DManager* sInstance;
};
