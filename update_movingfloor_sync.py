import sys
import re

# 1. MovingFloor.h
with open('Source/MovingFloor.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()

if '#include <list>' not in content:
    content = content.replace('#include <DxLib.h>', '#include <DxLib.h>\n#include <list>')

if 'static std::list<MovingFloor*> s_AllMovingFloors;' not in content:
    content = content.replace(
        'private:',
        'public:\n    static std::list<MovingFloor*> s_AllMovingFloors;\n\nprivate:\n    bool mIsSpedUp;'
    )

with open('Source/MovingFloor.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)

# 2. MovingFloor.cpp
with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# static リストの実体化
if 'std::list<MovingFloor*> MovingFloor::s_AllMovingFloors;' not in content:
    content = content.replace(
        '#include <cmath>',
        '#include <cmath>\n\nstd::list<MovingFloor*> MovingFloor::s_AllMovingFloors;'
    )

# コンストラクタ1の修正
content = content.replace(
    ', mpBlockMap(blockMap)\n{',
    ', mpBlockMap(blockMap)\n    , mIsSpedUp(false)\n{\n    s_AllMovingFloors.push_back(this);'
)

# コンストラクタ2の修正
content = content.replace(
    ', mpBlockMap(blockMap)\n{\n    //',
    ', mpBlockMap(blockMap)\n    , mIsSpedUp(false)\n{\n    s_AllMovingFloors.push_back(this);\n    //'
)

# コンストラクタ内での既存加速床チェック追加 (mVelocityの初期化後)
sync_code = """
    // もし同じ動き（Velocity）を持つ他の床がすでに加速済みなら、自分も最初から加速状態にする
    if (mFeature == FloorFeature::SpeedUpOnRide) {
        for (MovingFloor* floor : s_AllMovingFloors) {
            if (floor != this && floor->mIsSpedUp && 
                floor->mVelocity.x == this->mVelocity.x && floor->mVelocity.y == this->mVelocity.y) {
                mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
                mIsSpedUp = true;
                break;
            }
        }
    }
"""
content = re.sub(
    r"(mVelocity = VGet\(0, 0, 0\);\n    \})",
    r"\1" + sync_code,
    content
)
content = re.sub(
    r"(mHeight = static_cast<float>\(h\);\n\})",
    r"mHeight = static_cast<float>(h);\n" + sync_code + "}",
    content
)

# デストラクタ
content = content.replace(
    'MovingFloor::~MovingFloor()\n{',
    'MovingFloor::~MovingFloor()\n{\n    s_AllMovingFloors.remove(this);'
)

# CheckPlayerRiding の加速と連動
accel_code = """        // 乗った時に加速する機能（一度加速したら降りても戻らない、かつ同じ動きの他の床も連動する）
        if (mFeature == FloorFeature::SpeedUpOnRide && !mIsSpedUp) {
            mIsSpedUp = true;
            mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
            
            // 同じ方向・同じ速度の他の床も一斉に加速させる
            for (MovingFloor* floor : s_AllMovingFloors) {
                if (floor != this && floor->mFeature == FloorFeature::SpeedUpOnRide && 
                    floor->mVelocity.x == this->mVelocity.x && floor->mVelocity.y == this->mVelocity.y) {
                    floor->mCurrentSpeed = floor->mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
                    floor->mIsSpedUp = true;
                }
            }
        }"""

# 元の加速コードを削除して新しいものに置き換え
content = re.sub(
    r"        if \(mFeature == FloorFeature::SpeedUpOnRide\) \{\s*mCurrentSpeed = mBaseSpeed \* MovingFloorConstants::SpeedUpMultiplier;\s*\}",
    accel_code,
    content
)

# 降りたときのリセットを削除
reset_code_pattern = r"        //[^\n]*\n        if \(mFeature == FloorFeature::SpeedUpOnRide\) \{\s*mCurrentSpeed = mBaseSpeed;\s*\}"
content = re.sub(reset_code_pattern, "", content)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

