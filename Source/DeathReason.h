#pragma once

class DeathReason
{
public:

	// どうやって死亡したかの判別
	enum class DeathType
	{
		None,
		// ここからExcel
		WallHit,
		EnemyHit,
		Fall,
		NeedleTrapHit,
		DeathBlock,
	};

	// 死亡原因と番号をセットするための構造体
	struct DeathTutorialSetting
	{
		DeathType type;
		int startID;
	};

	// 死亡後に始めるIDの取得
	int GetDeathStartID(DeathType type) const;
};