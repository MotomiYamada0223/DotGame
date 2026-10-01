import sys

try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# Updateをシンプルにする
old_update = """void NeedleTrap::Update()
{
    // EnemyMoveでトラップ専用の動きを行う
    if (mpBlockMap != nullptr)
    {
        EnemyMove(*mpBlockMap);
    }

    // 更新された座標で当たり判定を更新
    // UpdateRect();
}"""
new_update = """void NeedleTrap::Update()
{
    Enemy::Update();
}"""
content = content.replace(old_update, new_update)

# EnemyMoveの冒頭に mpBlockMap = &blockMap; を追加
old_enemymove = """void NeedleTrap::EnemyMove(BlockMap& blockMap)
{
    if (mState == TrapState::Finished) return;"""
new_enemymove = """void NeedleTrap::EnemyMove(BlockMap& blockMap)
{
    mpBlockMap = &blockMap;
    if (mState == TrapState::Finished) return;"""
content = content.replace(old_enemymove, new_enemymove)

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

