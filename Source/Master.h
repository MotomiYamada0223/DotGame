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
	bool stageCleared[3];
	int deathCounts[3];
};


class Master
{
public:

	static ObjectManager* mpObjectManager;
	static GameManager* mpGameManager; // ゲームマネージャーのポインタ

	static const int MaxStageCount = 3;


	static bool gStageCleared[3];
	static int gDeathCounts[3];
	static int gTotalDeathCounts[3];

	static int gCurrentStageRunDeathCount;
	static int gCurrentStageIndex;

	static int gPlayerLive;

};
