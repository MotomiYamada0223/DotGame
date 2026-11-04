
#pragma once

#include "UnitStatus.h"
#include "BlockMap.h"
#include "Background.h"
#include "Player.h"
#include "TutorialTextManager.h"
#include "LeafCutter.h"
#include "RollingBoulder.h"
#include "Minecart.h"


/// <summary>
/// 各ステージごとに読み込むオブジェクトを設定するクラス
/// </summary>
class GameLoadData
{
public:
    GameLoadData(
        Player*& player,
        BlockMap& blockMap,
        Background& background,
        TutorialTextManager& tutorialText,
        GameProgress& progress,
        RollingBoulder& rollBoulder,
        Minecart& cart
    );

    void LoadInitialize(int stageNumber);
    void LoadUpdate();
    void LoadDraw();


    void DebugUpdate();
    void DebugDraw();

private:
    Player*& mpPlayer;
    GameProgress& mProgress;

    BlockMap& mBlockMap;
    Background& mBackground;
    TutorialTextManager& mTutorialText;
    RollingBoulder& mRollingBoulder;
    Minecart& mCart;



    // 各ステージで読み込むための処理
    void StageForest();
    void StageCave();

    int mnLoadNumber = 0;
};
