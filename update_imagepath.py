import sys
import re

# 1. MovingFloor.h
with open('Source/MovingFloor.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()
if '<string>' not in content:
    content = content.replace('#include <DxLib.h>', '#include <DxLib.h>\n#include <string>')
content = content.replace(
    'Player* player, BlockMap* blockMap);',
    'Player* player, BlockMap* blockMap, std::string imagePath = "");'
)
with open('Source/MovingFloor.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)

# 2. MovingFloor.cpp
with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# コンストラクタ定義1
content = content.replace(
    'Player* player, BlockMap* blockMap)',
    'Player* player, BlockMap* blockMap, std::string imagePath)'
)

# 画像ロード部分を置換（両方のコンストラクタにある）
old_load = """    // 画像の読み込み
    mImageHandle = LoadGraph(CharacterGraphPath::MoveFloor.c_str());"""

new_load = """    // 画像パスが指定されていない場合はデフォルトを使用
    std::string path = imagePath.empty() ? CharacterGraphPath::MoveFloor : imagePath;
    mImageHandle = LoadGraph(path.c_str());"""

# 日本語コメントが文字化けしている可能性があるので正規表現
content = re.sub(r"\s*mImageHandle = LoadGraph\(CharacterGraphPath::MoveFloor\.c_str\(\)\);", '\n' + new_load, content)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

# 3. FloorSpawner.h
with open('Source/FloorSpawner.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()
if '<string>' not in content:
    content = content.replace('#include <DxLib.h>', '#include <DxLib.h>\n#include <string>')
content = content.replace(
    'Player* player, BlockMap* blockMap);',
    'Player* player, BlockMap* blockMap, std::string imagePath = "");'
)
if 'std::string mImagePath;' not in content:
    content = content.replace('int mTimer;', 'int mTimer;\n    std::string mImagePath;')
with open('Source/FloorSpawner.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)

# 4. FloorSpawner.cpp
with open('Source/FloorSpawner.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()
content = content.replace(
    'Player* player, BlockMap* blockMap)',
    'Player* player, BlockMap* blockMap, std::string imagePath)'
)
content = content.replace(
    ', mpBlockMap(blockMap)',
    ', mpBlockMap(blockMap)\n    , mImagePath(imagePath)'
)
content = content.replace(
    'mpBlockMap\n        );',
    'mpBlockMap,\n            mImagePath\n        );'
)
with open('Source/FloorSpawner.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

# 5. TestScene.cpp
with open('Source/TestScene.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()
content = content.replace(
    'mpPlayer,\n\t\t&mBlockMap\n\t);',
    'mpPlayer,\n\t\t&mBlockMap,\n\t\tCharacterGraphPath::MoveFloor // ここで画像パスを指定\n\t);'
)
content = content.replace(
    'FloorFeature::Normal, mpPlayer, &mBlockMap\n\t);',
    'FloorFeature::Normal, mpPlayer, &mBlockMap, CharacterGraphPath::MoveFloor\n\t);'
)
with open('Source/TestScene.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

