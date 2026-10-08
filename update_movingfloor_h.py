import sys
import re

with open('Source/MovingFloor.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# BlockMap前方宣言追加
if 'class BlockMap;' not in content:
    content = content.replace('class Player;', 'class Player;\nclass BlockMap;')

# コンストラクタ修正
content = content.replace(
    'MovingFloor(VECTOR startPos, VECTOR endPos, float speed, MovePattern pattern, FloorFeature feature, Player* player);',
    'MovingFloor(VECTOR startPos, VECTOR endPos, float speed, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap);'
)
content = content.replace(
    'MovingFloor(VECTOR startPos, VECTOR velocity, MovePattern pattern, FloorFeature feature, Player* player);',
    'MovingFloor(VECTOR startPos, VECTOR velocity, MovePattern pattern, FloorFeature feature, Player* player, BlockMap* blockMap);'
)

# メンバ変数追加
if 'BlockMap* mpBlockMap;' not in content:
    content = content.replace('Player* mpPlayer;', 'Player* mpPlayer;\n    BlockMap* mpBlockMap;')

with open('Source/MovingFloor.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)
