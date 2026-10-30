#pragma once
#include "Scene.h"
#include "TutorialTextManager.h"
#include "Background.h"
#include "DrawFrame.h"
#include "Saint.h"
#include "BossIntroDirector.h"
#include "StageClear.h"
#include "CircleFade.h"
#include "LeafCutter.h"
#include "GameLoadData.h"
#include "RollingBoulder.h"
#include "Minecart.h"
#include "HotSpring.h"

// 前方宣言
class Player;
class CircleFade;

class GameScene : public Scene
{

public:

	GameScene();

	virtual ~GameScene() override;

	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Finalize() override;

	void SetTextUpdate(); // チュートリアルテキストの更新


private:
	GameLoadData mLoadData;

	Player* mpPlayer;

	TutorialTextManager mTutorialText;
	CircleFade mCircleFade;
	LeafCutter mLeaf;

	GameProgress mProgress; // プレイヤーの座標を参照するために保持
	BlockMap mBlockMap;
	Background mBackground; // スクロール背景
	DrawFrame mDrawFrame; //  天使のフレーム描画用
	Saint mSaint; // 天使のキャラクター画像
	BossIntroDirector mBossIntro;
	StageClear mStageClear; // ボス戦開始演出ディレクター
	RollingBoulder mRollingBoulder;
	Minecart mCart;
	HotSpring mHotSpring;

	bool mbIsLoaded = false; // マップがロードされたかどうかのフラグ
	bool mbWasPlayerDead = false; // 前フレームのプレイヤーの死亡状態を保持するフラグ

	// 円の描画
	bool mbCircleFlag;
};
