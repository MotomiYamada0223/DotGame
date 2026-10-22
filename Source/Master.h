#pragma once

#include "ObjectManager.h"
#include "GameManager.h"


//
// ゲーム内でひとつだけしか存在しないものを管理するクラス
// 例えば、
// SceneManager 　　だったり、（シーン管理クラス）
// SoundManager 　　だったり、（サウンド管理クラス）
// ResourceManager  だったり、（リソース（画像やモデル）管理クラス）
// を、どこからでも参照できるようにしておきたいので
// こういったクラスを作る
// 

struct SaveSlot {
	bool isUsed;
	bool stageCleared[4];
	int deathCounts[4];
};


class Master
{
public:

	static ObjectManager* mpObjectManager;
	static GameManager* mpGameManager; // ゲームマネージャーのポインタ

	static const int MaxStageCount = 4;


	static bool gStageCleared[4];
	static int gDeathCounts[4];
	static int gTotalDeathCounts[4];

	static int gCurrentStageRunDeathCount;
	static int gCurrentStageIndex;

	static int gPlayerLive;

};
