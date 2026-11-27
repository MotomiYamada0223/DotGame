#include "Effekseer2DManager.h"
#include "EffekseerForDXLib.h"
#include "BlockMap.h"
#include "GameConstants.h"

Effekseer2DManager* Effekseer2DManager::sInstance = nullptr;

// =========================================================================
// 2D Effekseer エフェクト定義＆ステージ別使用箇所一覧
// =========================================================================
static const Effect2DInfo kEffect2DTable[static_cast<int>(Effect2DId::Count)] =
{
	{
		Effect2DId::Stage3_RoarWave,
		"RoarWave",
		"Resource/Effect/Pierre01/Pierre01/SonicBoom.efk",
		18.0f,
		Effect2DStage::Stage3,
		"Stage3 - CloneWindRoarAttack: 分身2体が風の刃を放つ常時展開魔法陣"
	},
	{
		Effect2DId::Stage3_WindTornado,
		"WindTornado",
		"Resource/Effect/Samples/00_Version16/ForceFieldTornado.efkefc",
		18.0f,
		Effect2DStage::Reserved,
		"Stage3（予備：風渦オーラ用 / 現在未使用）"
	},
	{
		Effect2DId::Stage3_FlameBurst,
		"FlameBurst",
		"Resource/Effect/tktk01/tktk01/Sword1.efk",
		14.0f,
		Effect2DStage::Stage3,
		"Stage3 - CloneDashAttack / CloneMeteorDashAttack: 究極竜本体 突進斬撃バースト"
	},
	{
		Effect2DId::Stage3_HolyStorm,
		"HolyStorm",
		"Resource/Effect/tktk01/tktk01/Blow5.efk",
		16.0f,
		Effect2DStage::Stage3,
		"Stage3 - Stage3BossMovie / RoarAttack / CloneWindRoarAttack: 黄金覚醒バースト・咆哮衝撃バースト"
	},
	{
		Effect2DId::Stage3_Explosion,
		"Explosion",
		"Resource/Effect/Pierre01/Pierre01/HolySandstorm.efk",
		24.0f,
		Effect2DStage::Reserved,
		"Stage3（予備：爆風用 / 現在未使用）"
	},
	{
		Effect2DId::Stage3_Thunder,
		"Thunder",
		"Resource/Effect/NextSoft01/NextSoft01/Sword2.efk",
		14.0f,
		Effect2DStage::Stage3,
		"Stage3 - UltimateWindSlamAttack / BossMovie / CloneDash: 電撃・突進スパーク"
	},
	{
		Effect2DId::Stage3_SlashWind,
		"SlashWind",
		"Resource/Effect/tktk01/tktk01/Sword5.efk",
		15.0f,
		Effect2DStage::Stage3,
		"Stage3 - UltimateWindSlamAttack / CloneDash / CloneWindRoarAttack: 風斬撃・分身風の刃（当たり判定付き）"
	},
	{
		Effect2DId::Stage3_Aura,
		"Aura",
		"Resource/Effect/Samples/00_Version16/Aura01.efkefc",
		18.0f,
		Effect2DStage::Reserved,
		"Stage3（予備：オーラ渦用 / 現在未使用）"
	},
	{
		Effect2DId::Stage3_HitLight,
		"HitLight",
		"Resource/Effect/Samples/02_Tktk03/Light.efkefc",
		14.0f,
		Effect2DStage::Stage3,
		"Stage3 - CloneWindRoarAttack: 反射・ヒット閃光"
	},
	{
		Effect2DId::Stage3_MagicCircle,
		"MagicCircle",
		"Resource/Effect/NextSoft01/NextSoft01/MagicShield.efk",
		16.0f,
		Effect2DStage::Stage3,
		"Stage3 - CloneWindRoarAttack: 分身2体が風の刃を放つ常時展開魔法陣"
	}
};

Effekseer2DManager::Effekseer2DManager()
{
	for (int i = 0; i < static_cast<int>(Effect2DId::Count); ++i)
	{
		mResourceHandles[i] = -1;
		mBaseScales[i]      = kEffect2DTable[i].baseScale;
	}
}

Effekseer2DManager::~Effekseer2DManager()
{
	Finalize();
}

const Effect2DInfo& Effekseer2DManager::GetInfo(Effect2DId id)
{
	int idx = static_cast<int>(id);
	if (idx < 0 || idx >= static_cast<int>(Effect2DId::Count))
	{
		idx = 0;
	}
	return kEffect2DTable[idx];
}

const Effect2DInfo* Effekseer2DManager::GetAllInfos(int& outCount)
{
	outCount = static_cast<int>(Effect2DId::Count);
	return kEffect2DTable;
}

void Effekseer2DManager::Initialize()
{
	Finalize();

	Effekseer_Set2DSetting(ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight);

	std::vector<int> warmupHandles;
	for (int i = 0; i < static_cast<int>(Effect2DId::Count); ++i)
	{
		const Effect2DInfo& info = kEffect2DTable[i];
		mBaseScales[i]      = info.baseScale;
		mResourceHandles[i] = LoadEffekseerEffect(info.path, 1.0f);
		if (mResourceHandles[i] != -1)
		{
			int h = PlayEffekseer2DEffect(mResourceHandles[i]);
			if (h != -1)
			{
				SetPosPlayingEffekseer2DEffect(h, -9999.0f, -9999.0f, 0.0f);
				warmupHandles.push_back(h);
			}
		}
	}
	if (!warmupHandles.empty())
	{
		UpdateEffekseer2D();
		for (int h : warmupHandles)
		{
			StopEffekseer2DEffect(h);
		}
	}

	sInstance = this;
}

int Effekseer2DManager::Play(Effect2DId id, float worldX, float worldY, float scaleMul, float speed, float rotZ, int r, int g, int b, int a)
{
	if (sInstance != nullptr)
	{
		return sInstance->PlayEffect(id, worldX, worldY, scaleMul, speed, rotZ, r, g, b, a);
	}
	return -1;
}

void Effekseer2DManager::SetTransform(int playingHandle, float worldX, float worldY, float scaleMul, float rotZ)
{
	if (sInstance != nullptr)
	{
		sInstance->UpdateEffectTransform(playingHandle, worldX, worldY, scaleMul, rotZ);
	}
}

void Effekseer2DManager::Stop(int playingHandle)
{
	if (sInstance != nullptr)
	{
		sInstance->StopEffect(playingHandle);
	}
}

bool Effekseer2DManager::IsPlaying(int playingHandle)
{
	if (sInstance != nullptr)
	{
		return sInstance->IsEffectPlaying(playingHandle);
	}
	return false;
}

int Effekseer2DManager::PlayEffect(Effect2DId id, float worldX, float worldY, float scaleMul, float speed, float rotZ, int r, int g, int b, int a)
{
	int idx = static_cast<int>(id);
	if (idx < 0 || idx >= static_cast<int>(Effect2DId::Count))
	{
		return -1;
	}

	int resHandle = mResourceHandles[idx];
	if (resHandle == -1)
	{
		return -1;
	}

	if (static_cast<int>(mActiveEffects.size()) >= MAX_ACTIVE_EFFECTS)
	{
		int oldH = mActiveEffects.front().playingHandle;
		SetScalePlayingEffekseer2DEffect(oldH, 0.0f, 0.0f, 0.0f);
		SetPosPlayingEffekseer2DEffect(oldH, -99999.0f, -99999.0f, 0.0f);
		StopEffekseer2DEffect(oldH);
		mActiveEffects.erase(mActiveEffects.begin());
	}

	int playingHandle = PlayEffekseer2DEffect(resHandle);
	if (playingHandle == -1)
	{
		return -1;
	}
	SetPosPlayingEffekseer2DEffect(playingHandle, -99999.0f, -99999.0f, 0.0f);

	float finalScale = mBaseScales[idx] * scaleMul;
	SetScalePlayingEffekseer2DEffect(playingHandle, finalScale, finalScale, finalScale);
	SetSpeedPlayingEffekseer2DEffect(playingHandle, speed);
	if (rotZ != 0.0f)
	{
		SetRotationPlayingEffekseer2DEffect(playingHandle, 0.0f, 0.0f, rotZ);
	}
	if (r != 255 || g != 255 || b != 255 || a != 255)
	{
		SetColorPlayingEffekseer2DEffect(playingHandle, r, g, b, a);
	}

	ActiveEffect2D active;
	active.playingHandle = playingHandle;
	active.id            = id;
	active.worldX        = worldX;
	active.worldY        = worldY;
	active.baseScale     = finalScale;
	mActiveEffects.push_back(active);
	return playingHandle;
}

void Effekseer2DManager::UpdateEffectTransform(int playingHandle, float worldX, float worldY, float scaleMul, float rotZ)
{
	if (playingHandle == -1) return;
	for (auto& ef : mActiveEffects)
	{
		if (ef.playingHandle == playingHandle)
		{
			ef.worldX = worldX;
			ef.worldY = worldY;
			if (scaleMul > 0.0f)
			{
				ef.baseScale = mBaseScales[static_cast<int>(ef.id)] * scaleMul;
			}
			if (rotZ != 0.0f)
			{
				SetRotationPlayingEffekseer2DEffect(ef.playingHandle, 0.0f, 0.0f, rotZ);
			}
			break;
		}
	}
}

void Effekseer2DManager::StopEffect(int playingHandle)
{
	if (playingHandle == -1) return;
	for (auto it = mActiveEffects.begin(); it != mActiveEffects.end(); ++it)
	{
		if (it->playingHandle == playingHandle)
		{
			SetScalePlayingEffekseer2DEffect(it->playingHandle, 0.0f, 0.0f, 0.0f);
			SetPosPlayingEffekseer2DEffect(it->playingHandle, -99999.0f, -99999.0f, 0.0f);
			StopEffekseer2DEffect(it->playingHandle);
			mActiveEffects.erase(it);
			break;
		}
	}
}

bool Effekseer2DManager::IsEffectPlaying(int playingHandle) const
{
	if (playingHandle == -1) return false;
	for (const auto& ef : mActiveEffects)
	{
		if (ef.playingHandle == playingHandle)
		{
			return (IsEffekseer2DEffectPlaying(playingHandle) == 0);
		}
	}
	return false;
}

void Effekseer2DManager::Update(const BlockMap& blockMap, int viewWidth, int viewHeight)
{
	UpdateWithScroll(
		static_cast<float>(blockMap.GetScrollX()),
		static_cast<float>(blockMap.GetScrollY()),
		viewWidth,
		viewHeight
	);
}

void Effekseer2DManager::UpdateWithScroll(float scrollX, float scrollY, int viewWidth, int viewHeight)
{
	if (mActiveEffects.empty())
	{
		return;
	}

	(void)viewHeight;
	float scaleRatio = (viewWidth > 0)
		? (static_cast<float>(ScreenSize::ScrrenWidth) / static_cast<float>(viewWidth))
		: 1.0f;

	for (auto it = mActiveEffects.begin(); it != mActiveEffects.end();)
	{
		if (IsEffekseer2DEffectPlaying(it->playingHandle) != 0)
		{
			it = mActiveEffects.erase(it);
		}
		else
		{
			float sx = (it->worldX - scrollX) * scaleRatio;
			float sy = (it->worldY - scrollY) * scaleRatio;
			float sc = it->baseScale * scaleRatio;
			SetPosPlayingEffekseer2DEffect(it->playingHandle, sx, sy, 0.0f);
			SetScalePlayingEffekseer2DEffect(it->playingHandle, sc, sc, sc);
			++it;
		}
	}

	UpdateEffekseer2D();
}

void Effekseer2DManager::Draw()
{
	if (mActiveEffects.empty())
	{
		return;
	}

	DrawEffekseer2D();
}

void Effekseer2DManager::StopAll()
{
	for (const auto& ef : mActiveEffects)
	{
		SetScalePlayingEffekseer2DEffect(ef.playingHandle, 0.0f, 0.0f, 0.0f);
		SetPosPlayingEffekseer2DEffect(ef.playingHandle, -99999.0f, -99999.0f, 0.0f);
		StopEffekseer2DEffect(ef.playingHandle);
	}
	mActiveEffects.clear();
}

void Effekseer2DManager::Finalize()
{
	StopAll();

	for (int i = 0; i < static_cast<int>(Effect2DId::Count); ++i)
	{
		if (mResourceHandles[i] != -1)
		{
			DeleteEffekseerEffect(mResourceHandles[i]);
			mResourceHandles[i] = -1;
		}
	}

	if (sInstance == this)
	{
		sInstance = nullptr;
	}
}
