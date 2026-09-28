$content = @"
#pragma once
#include "Enemy.h"
#include "UnitStatus.h"

class EnemySlime : public Enemy, public UnitStatus
{
public:
    EnemySlime(VECTOR initPos);
    virtual ~EnemySlime();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void EnemyMove(BlockMap& blockMap) override;
    virtual void UpdateStatusByProgress(GameProgress progress) override;

protected:
    float mMoveSpeed;
};
"@
[System.IO.File]::WriteAllText("Source\EnemySlime.h", $content, [System.Text.Encoding]::GetEncoding(932))
