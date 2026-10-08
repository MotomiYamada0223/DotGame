import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# コンストラクタ1修正
content = content.replace(
    'MovingFloor::MovingFloor(VECTOR startPos, VECTOR endPos, float speed, MovePattern pattern, FloorFeature feature, Player* player)',
    'MovingFloor::MovingFloor(VECTOR startPos, VECTOR endPos, float speed, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap)'
)
content = content.replace(
    ', mIsPlayerRiding(false)',
    ', mIsPlayerRiding(false)\n    , mpBlockMap(blockMap)'
)

# コンストラクタ2修正 (正規表現を使わずに確実に置換するため、2つ目のmIsPlayerRiding(false)の置換が必要。上ので一緒に置換される)
content = content.replace(
    'MovingFloor::MovingFloor(VECTOR startPos, VECTOR velocity, MovePattern pattern, FloorFeature feature, Player* player)',
    'MovingFloor::MovingFloor(VECTOR startPos, VECTOR velocity, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap)'
)

# Drawメソッド修正
old_draw = """void MovingFloor::Draw()
{
    if (IsDeleteFlag()) return;

    VECTOR pos = GetPosition();
    DrawRotaGraph(static_cast<int>(pos.x), static_cast<int>(pos.y), 1.0, 0.0, mImageHandle, TRUE);
}"""

new_draw = """void MovingFloor::Draw()
{
    if (IsDeleteFlag()) return;

    VECTOR pos = GetPosition();
    float screenX = ConvertToScreenX(pos.x, mpBlockMap);
    
    DrawRotaGraph(static_cast<int>(screenX), static_cast<int>(pos.y), 1.0, 0.0, mImageHandle, TRUE);
}"""
content = content.replace(old_draw, new_draw)

# 乗った時のプレイヤー速度リセット追加
# CheckPlayerRiding内の mIsPlayerRiding = true; の直後に重力リセットを入れる
old_ride = """    if (inX && inY) {
        mIsPlayerRiding = true;
        
        // y@\z"""
new_ride = """    if (inX && inY) {
        mIsPlayerRiding = true;
        mpPlayer->SetVelocityY(0.0f); // 重力が蓄積してすり抜けないようにリセット
        
        // y@\z"""
content = content.replace(old_ride, new_ride)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

