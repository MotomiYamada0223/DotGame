#pragma once
#include "Effekseer2DManager.h"

class BlockMap;

// =========================================================================
// Stage 3 向けの 2D Effekseer 互換ラッパー
// 実体およびステージ別の使用箇所一覧は Effekseer2DManager.h / .cpp で一元管理
// =========================================================================
enum class Stage3EffectType
{
	RoarWave = 0,  // -> Effect2DId::Stage3_RoarWave
	WindTornado,   // -> Effect2DId::Stage3_WindTornado
	FlameBurst,    // -> Effect2DId::Stage3_FlameBurst
	HolyStorm,     // -> Effect2DId::Stage3_HolyStorm
	Explosion,     // -> Effect2DId::Stage3_Explosion
	Thunder,       // -> Effect2DId::Stage3_Thunder
	SlashWind,     // -> Effect2DId::Stage3_SlashWind
	Aura,          // -> Effect2DId::Stage3_Aura
	HitLight,      // -> Effect2DId::Stage3_HitLight
	Count
};

class Stage3EffectManager
{
public:
	Stage3EffectManager() = default;
	~Stage3EffectManager() = default;

	void Initialize() { mManager.Initialize(); }
	void Update(const BlockMap& blockMap, int viewWidth, int viewHeight) { mManager.Update(blockMap, viewWidth, viewHeight); }
	void Draw() { mManager.Draw(); }
	void StopAll() { mManager.StopAll(); }
	void Finalize() { mManager.Finalize(); }

	int  PlayEffect(Stage3EffectType type, float worldX, float worldY, float scaleMul = 1.0f, float speed = 1.0f, float rotZ = 0.0f, int r = 255, int g = 255, int b = 255, int a = 255);
	void UpdateEffectTransform(int playingHandle, float worldX, float worldY, float scaleMul = -1.0f, float rotZ = 0.0f);
	void StopEffect(int playingHandle);
	bool IsEffectPlaying(int playingHandle) const;

	static int  Play(Stage3EffectType type, float worldX, float worldY, float scaleMul = 1.0f, float speed = 1.0f, float rotZ = 0.0f, int r = 255, int g = 255, int b = 255, int a = 255);
	static void SetTransform(int playingHandle, float worldX, float worldY, float scaleMul = -1.0f, float rotZ = 0.0f);
	static void Stop(int playingHandle);
	static bool IsPlaying(int playingHandle);

private:
	Effekseer2DManager mManager;
};
