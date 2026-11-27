#include "Stage3EffectManager.h"

namespace
{
	Effect2DId ToEffect2DId(Stage3EffectType type)
	{
		return static_cast<Effect2DId>(static_cast<int>(type));
	}
}

int Stage3EffectManager::Play(Stage3EffectType type, float worldX, float worldY, float scaleMul, float speed, float rotZ, int r, int g, int b, int a)
{
	return Effekseer2DManager::Play(ToEffect2DId(type), worldX, worldY, scaleMul, speed, rotZ, r, g, b, a);
}

void Stage3EffectManager::SetTransform(int playingHandle, float worldX, float worldY, float scaleMul, float rotZ)
{
	Effekseer2DManager::SetTransform(playingHandle, worldX, worldY, scaleMul, rotZ);
}

void Stage3EffectManager::Stop(int playingHandle)
{
	Effekseer2DManager::Stop(playingHandle);
}

bool Stage3EffectManager::IsPlaying(int playingHandle)
{
	return Effekseer2DManager::IsPlaying(playingHandle);
}

int Stage3EffectManager::PlayEffect(Stage3EffectType type, float worldX, float worldY, float scaleMul, float speed, float rotZ, int r, int g, int b, int a)
{
	return mManager.PlayEffect(ToEffect2DId(type), worldX, worldY, scaleMul, speed, rotZ, r, g, b, a);
}

void Stage3EffectManager::UpdateEffectTransform(int playingHandle, float worldX, float worldY, float scaleMul, float rotZ)
{
	mManager.UpdateEffectTransform(playingHandle, worldX, worldY, scaleMul, rotZ);
}

void Stage3EffectManager::StopEffect(int playingHandle)
{
	mManager.StopEffect(playingHandle);
}

bool Stage3EffectManager::IsEffectPlaying(int playingHandle) const
{
	return mManager.IsEffectPlaying(playingHandle);
}
