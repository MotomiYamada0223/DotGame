#include "DeathReason.h"

namespace
{
	// 死亡原因と死亡チュートリアルの開始ID
	constexpr DeathReason::DeathTutorialSetting DeathTutorialSettings[] =
	{
		{ DeathReason::DeathType::WallHit,		 1 },
		{ DeathReason::DeathType::EnemyHit,		 2 },
		{ DeathReason::DeathType::Fall,			 3 },
		{ DeathReason::DeathType::NeedleTrapHit, 4 },
		{ DeathReason::DeathType::DeathBlock, 5 },
	};
}


// 死亡理由から死亡チュートリアルの開始IDを取得
int DeathReason::GetDeathStartID(DeathType type) const
{
	for (const auto& setting : DeathTutorialSettings)
	{
		if (setting.type == type)
		{
			return setting.startID;
		}
	}

	// 対応する死亡理由がなかった場合
	return 0;
}